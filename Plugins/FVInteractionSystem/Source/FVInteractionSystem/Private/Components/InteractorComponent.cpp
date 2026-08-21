#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
#include "Core/InteractionRequirement.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractorComponent)

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	Registry = GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>();

	bIsInitialized = Registry != nullptr;
	SetComponentTickEnabled(bIsInitialized);
}


void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsInitialized)
	{
		TimeSinceLastUpdate += DeltaTime;

		if (DetectionUpdateInterval <= 0.f || TimeSinceLastUpdate >= DetectionUpdateInterval)
		{
			TimeSinceLastUpdate = 0.f;
			DetectInteractables();
			RefreshOffers(false);
		}
	}
}

bool UInteractorComponent::TryExecuteAction(FGameplayTag InputTag)
{
	UInteractableComponent* Target = FocusedTarget.Get();

	if (!Target)
	{
		return false;
	}

	const FInteractionPrompt* Prompt = CachedPrompts.FindByPredicate(
		[&InputTag](const FInteractionPrompt& Candidate) { return Candidate.InputTag.MatchesTagExact(InputTag); });

	if (!Prompt)
	{
		return false;
	}

	if (Prompt->IsEnabled())
	{
		const FGameplayTag ActionTag = Prompt->ActionTag;

		if (ExecuteAction.IsBound())
		{
			ExecuteAction.Execute(ActionTag, MakeContext(*Target));
		}

		RefreshOffers();
	}

	return true;
}

FInteractionContext UInteractorComponent::MakeContext(const UInteractableComponent& Target) const
{
	FInteractionContext Context;
	Context.Interactor = GetOwner();
	Context.Target = Target.GetOwner();
	Context.InteractionPoint = Target.GetAimProbeLocation();
	return Context;
}

bool UInteractorComponent::ResolveAvailability(const FInteractionOffer& Offer, bool& bOutHidden) const
{
	bOutHidden = false;

	FInteractionResolveContext Context;
	Context.Interactor = GetOwner();
	Context.Interactable = FocusedTarget.Get();
	Context.ActionTag = Offer.ActionTag;

	auto Evaluate = [&Context, &bOutHidden](const TArray<TObjectPtr<UInteractionRequirement>>& Requirements)
	{
		for (const UInteractionRequirement* Requirement : Requirements)
		{
			if (Requirement && !Requirement->IsMet(Context))
			{
				bOutHidden = Requirement->Gate == EInteractionGate::Hide;
				return false;
			}
		}
		return true;
	};

	return Evaluate(GlobalRequirements) && Evaluate(Offer.Requirements);
}

void UInteractorComponent::DetectInteractables()
{
	FVector ViewLocation = Owner->GetActorLocation();
	FVector ViewForward = Owner->GetActorForwardVector();

	if (const APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
	{
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
		ViewForward = ViewRotation.Vector();
	}

	Registry->QueryInRange(ViewLocation, MaxDetectionRadius, Candidates);

	UInteractableComponent* BestCandidate = nullptr;
	float BestScore = -1.f;

	for (UInteractableComponent* Candidate : Candidates)
	{
		const FInteractionFocusProfile& Profile = Candidate->GetFocusProfile();
		const FVector ToTarget = Candidate->GetAimProbeLocation() - ViewLocation;
		const float Distance = ToTarget.Size();

		if (Distance > Profile.DetectionRadius)
		{
			continue;
		}

		const float Dot = FVector::DotProduct(ViewForward, ToTarget.GetSafeNormal());
		if (Dot < Profile.ConeCosine)
		{
			continue;
		}

		const float AngularRange = 1.f - Profile.ConeCosine;
		const float AngularQuality = AngularRange > KINDA_SMALL_NUMBER
			? FMath::Clamp((Dot - Profile.ConeCosine) / AngularRange, 0.f, 1.f)
			: 1.f;

		const float DistanceQuality = Profile.DetectionRadius > KINDA_SMALL_NUMBER
			? FMath::Clamp(1.f - Distance / Profile.DetectionRadius, 0.f, 1.f)
			: 0.f;

		float Score = Profile.AngularWeight * AngularQuality + Profile.DistanceWeight * DistanceQuality;

		if (Candidate == FocusedTarget.Get())
		{
			Score += StickyFocusBonus;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestCandidate = Candidate;
		}
	}

	SetFocusedTarget(BestCandidate);
}

void UInteractorComponent::SetFocusedTarget(UInteractableComponent* NewTarget)
{
	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}

	if (UInteractableComponent* Previous = FocusedTarget.Get())
	{
		Previous->SetFocused(false);
	}

	FocusedTarget = NewTarget;

	if (NewTarget)
	{
		NewTarget->SetFocused(true);
	}

	OnFocusChanged.Broadcast(NewTarget);
	RefreshOffers();
}

void UInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UInteractableComponent* Target = FocusedTarget.Get();
	if (!Target)
	{
		return;
	}

	TArray<FInteractionPrompt> NewPrompts;

	for (const TPair<FGameplayTag, FInteractionOffer>& Pair : Target->GetOffers())
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		bool bHidden = false;
		const bool bMet = ResolveAvailability(Pair.Value, bHidden);

		if (!bMet && bHidden)
		{
			continue;
		}

		FInteractionPrompt& Prompt = NewPrompts.AddDefaulted_GetRef();
		Prompt.InputTag = Pair.Key;
		Prompt.ActionTag = Pair.Value.ActionTag;
		Prompt.bEnabled = bMet;
	}

	NewPrompts.Sort([](const FInteractionPrompt& A, const FInteractionPrompt& B)
	{
		return A.InputTag.ToString() < B.InputTag.ToString();
	});

	const bool bChanged = NewPrompts != CachedPrompts;
	CachedPrompts = MoveTemp(NewPrompts);

	if (bChanged || bForceBroadcast)
	{
		OnOffersChanged.Broadcast(CachedPrompts);
	}
}