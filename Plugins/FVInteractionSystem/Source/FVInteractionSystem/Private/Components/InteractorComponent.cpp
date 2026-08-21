#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
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
			RevalidatePrompts();
		}
	}
}

EInteractionResult UInteractorComponent::TryExecuteAction(FGameplayTag InputTag)
{
	UInteractableComponent* Target = FocusedTarget.Get();

	if (!Target)
	{
		return EInteractionResult::NoInteractable;
	}

	const FInteractionPrompt* Prompt = CachedPrompts.FindByPredicate(
		[&InputTag](const FInteractionPrompt& Candidate) { return Candidate.InputTag.MatchesTagExact(InputTag); });

	if (!Prompt)
	{
		return EInteractionResult::NoInteractable;
	}

	switch (Prompt->Result.Availability)
	{
	case EInteractionAvailability::RequirementNotMet:
		return EInteractionResult::RequirementNotMet;
	case EInteractionAvailability::Blocked:
		return EInteractionResult::Blocked;
	default:
		break;
	}

	const FGameplayTag ActionTag = Prompt->ActionTag;

	if (ExecuteAction.IsBound() && !ExecuteAction.Execute(ActionTag, MakeContext(*Target)))
	{
		return EInteractionResult::Blocked;
	}

	RefreshOffers();
	return EInteractionResult::Success;
}

FInteractionContext UInteractorComponent::MakeContext(const UInteractableComponent& Target) const
{
	FInteractionContext Context;
	Context.Interactor = GetOwner();
	Context.Target = Target.GetOwner();
	Context.InteractionPoint = Target.GetAimProbeLocation();
	return Context;
}

FInteractionAvailabilityResult UInteractorComponent::ResolveAvailability(const FGameplayTag& ActionTag) const
{
	if (ResolveAction.IsBound())
	{
		return ResolveAction.Execute(ActionTag);
	}

	FInteractionAvailabilityResult Result;
	Result.Availability = EInteractionAvailability::Available;
	return Result;
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

void UInteractorComponent::RefreshOffers()
{
	CachedPrompts.Reset();

	if (const UInteractableComponent* Target = FocusedTarget.Get())
	{
		for (const TPair<FGameplayTag, FInteractionOffer>& Pair : Target->GetOffers())
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			FInteractionPrompt& Prompt = CachedPrompts.AddDefaulted_GetRef();
			Prompt.InputTag = Pair.Key;
			Prompt.ActionTag = Pair.Value.ActionTag;
			Prompt.Result = ResolveAvailability(Pair.Value.ActionTag);
		}

		CachedPrompts.Sort([](const FInteractionPrompt& A, const FInteractionPrompt& B)
		{
			return A.InputTag.ToString() < B.InputTag.ToString();
		});
	}

	OnOffersChanged.Broadcast(CachedPrompts);
}

void UInteractorComponent::RevalidatePrompts()
{
	if (CachedPrompts.IsEmpty())
	{
		return;
	}

	bool bChanged = false;

	for (FInteractionPrompt& Prompt : CachedPrompts)
	{
		const FInteractionAvailabilityResult NewResult = ResolveAvailability(Prompt.ActionTag);

		if (NewResult != Prompt.Result)
		{
			Prompt.Result = NewResult;
			bChanged = true;
		}
	}

	if (bChanged)
	{
		OnOffersChanged.Broadcast(CachedPrompts);
	}
}