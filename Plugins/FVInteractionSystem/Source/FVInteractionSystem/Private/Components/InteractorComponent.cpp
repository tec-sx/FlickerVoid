#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
#include "FVInteractionSystemSettings.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

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
		}
	}
}

EInteractionResult UInteractorComponent::TryExecuteAction(FGameplayTag InputTag)
{
	UInteractableComponent* Target = FocusedTarget.Get();
	UAbilitySystemComponent* ASC = GetOwnerASC();

	if (!Target || !ASC)
	{
		return EInteractionResult::NoInteractable;
	}

	const FInteractionOffer* Offer = Target->FindOffer(InputTag);
	if (!Offer || !Offer->IsValid())
	{
		return EInteractionResult::NoInteractable;
	}

	if (!HasAbilityForAction(*ASC, Offer->ActionTag))
	{
		return EInteractionResult::RequirementNotMet;
	}

	FGameplayEventData Payload;
	Payload.EventTag = Offer->ActionTag;
	Payload.Instigator = GetOwner();
	Payload.Target = Target->GetOwner();
	Payload.OptionalObject = Target;

	if (ASC->HandleGameplayEvent(Offer->ActionTag, &Payload) == 0)
	{
		return EInteractionResult::Blocked;
	}

	RefreshOffers();
	return EInteractionResult::Success;
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
		const FInteractionFocusProfile Profile = Candidate->GetFocusProfile();
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

bool UInteractorComponent::HasAbilityForAction(const UAbilitySystemComponent& ASC, const FGameplayTag& ActionTag) const
{
	for (const FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetAssetTags().HasTag(ActionTag))
		{
			return true;
		}
	}

	return false;
}

bool UInteractorComponent::CanActivateAction(const UAbilitySystemComponent& ASC, const FGameplayTag& ActionTag) const
{
	for (const FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		if (!Spec.Ability || !Spec.Ability->GetAssetTags().HasTag(ActionTag))
		{
			continue;
		}

		const FGameplayAbilityActorInfo* ActorInfo = ASC.AbilityActorInfo.Get();
		if (Spec.Ability->CanActivateAbility(Spec.Handle, ActorInfo))
		{
			return true;
		}
	}

	return false;
}

UAbilitySystemComponent* UInteractorComponent::GetOwnerASC() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
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

	const UInteractableComponent* Target = FocusedTarget.Get();
	const UAbilitySystemComponent* ASC = GetOwnerASC();

	if (Target && ASC)
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

			if (!HasAbilityForAction(*ASC, Pair.Value.ActionTag))
			{
				Prompt.Availability = EInteractionAvailability::RequirementNotMet;
			}
			else
			{
				Prompt.Availability = CanActivateAction(*ASC, Pair.Value.ActionTag)
					? EInteractionAvailability::Available
					: EInteractionAvailability::Blocked;
			}

		}

		CachedPrompts.Sort([](const FInteractionPrompt& A, const FInteractionPrompt& B)
		{
			return A.InputTag.ToString() < B.InputTag.ToString();
		});
	}

	OnOffersChanged.Broadcast(CachedPrompts);
}