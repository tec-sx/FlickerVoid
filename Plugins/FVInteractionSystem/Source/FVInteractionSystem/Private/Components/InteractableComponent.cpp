#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Core/InteractionTags.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include <Subsystems/InteractionRegistrySubsystem.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableComponent)

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(GetOwner()))
	{
		return;
	}

	const FInteractableSettings& Defaults = UFVInteractionSystemSettings::Get().InteractableBaseSettings;

	if (DetectionRadius < 0.f)
	{
		DetectionRadius = Defaults.DefaultDetectionRadius;
	}

	if (!Type.IsValid())
	{
		Type = Defaults.InteractableMainTag;
	}

	if (CooldownPeriod < 0.f)
	{
		CooldownPeriod = Defaults.DefaultCooldownPeriod;
	}

	State = Defaults.DefaultInteractableState;

	bIsInitialized = true;

	if (!FocusComponentTag.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' has an InteractableComponent but no FocusComponentTag value is set. It will not be registered as interactable."),
			*GetOwner()->GetName());
		return;
	}
	
	FocusPrimitive = GetOwner()->FindComponentByTag<UPrimitiveComponent>(FocusComponentTag);

	if (!FocusPrimitive.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' has an InteractableComponent but no PrimitiveComponent with the set tag value was found to derive focus bounds from. It will not be registered as interactable."),
			*GetOwner()->GetName());
		return;
	}

	FocusPrimitive->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

	if (UInteractionRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>())
	{
		Registry->Register(this);
	}
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimer);

		if (UInteractionRegistrySubsystem* Registry = World->GetSubsystem<UInteractionRegistrySubsystem>())
		{
			Registry->Unregister(this);
		}
	}
	
	bIsInitialized = false;

	Super::EndPlay(EndPlayReason);
}

FVector UInteractableComponent::GetFocusPoint() const
{
	if (const UPrimitiveComponent* Primitive = FocusPrimitive.Get())
	{
		return Primitive->Bounds.Origin;
	}

	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

void UInteractableComponent::SetFocused(bool bFocused, UInteractorComponent* Interactor)
{
	if (!bIsInitialized || bFocused == bIsInFocus)
	{
		return;
	}

	bIsInFocus = bFocused;
	FocusStateChanged.Broadcast(bIsInFocus, Interactor);
}

bool UInteractableComponent::IsTransitionAllowed(EInteractableState From, EInteractableState To)
{
	auto LogRejection = [&From, &To](const TCHAR* Reason)
	{
		UE_LOG(LogFVInteraction, Verbose, TEXT("Rejected %s -> %s: %s"),
			*UEnum::GetValueAsString(From),
			*UEnum::GetValueAsString(To),
			Reason);
	};
	
	if (To == EInteractableState::Default)
	{
		LogRejection(TEXT("Default is not a valid target state."));
		return false;
	}

	if (From == To)
	{
		LogRejection(TEXT("Already in the requested state."));
		return false;
	}

	if (From == EInteractableState::Completed)
	{
		LogRejection(TEXT("Completed is terminal."));
		return false;
	}

	switch (To)
	{
	case EInteractableState::Awake:
		if (From == EInteractableState::Interacting)
		{
			LogRejection(TEXT("Finish or cancel the interaction before waking."));
			return false;
		}
		break;

	case EInteractableState::Interacting:
		if (From != EInteractableState::Awake && From != EInteractableState::Paused)
		{
			LogRejection(TEXT("Only an Awake or Paused interactable can start interacting."));
			return false;
		}
		break;

	case EInteractableState::Paused:
		if (From != EInteractableState::Interacting)
		{
			LogRejection(TEXT("Only an interacting interactable can be paused."));
			return false;
		}
		break;

	case EInteractableState::Cooldown:
	case EInteractableState::Completed:
		if (From != EInteractableState::Interacting && From != EInteractableState::Awake)
		{
			LogRejection(TEXT("Only an interacting or awake interactable can finish."));
			return false;
		}
		break;

	default:
		break;
	}

	return true;
}

bool UInteractableComponent::SetState(EInteractableState NewState)
{
	if (!IsTransitionAllowed(State, NewState))
	{
		return false;
	}

	const EInteractableState OldState = State;
	State = NewState;

	ApplyStateTag(OldState, NewState);
	StateChanged.Broadcast(NewState);
	ProcessDependencies();

	return true;
}

void UInteractableComponent::ApplyStateTag(const EInteractableState OldState, const EInteractableState NewState) const
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	const FGameplayTag OldTag = InteractionTags::StateToTag(OldState);
	const FGameplayTag NewTag = InteractionTags::StateToTag(NewState);

	if (OldTag.IsValid())
	{
		OwningActor->Tags.Remove(OldTag.GetTagName());
	}

	if (NewTag.IsValid())
	{
		OwningActor->Tags.AddUnique(NewTag.GetTagName());
	}
}

void UInteractableComponent::ConsumeOffer(const FGameplayTag& ActionTag)
{
	FInteractionOffer* Offer = Offers.FindByPredicate([&ActionTag](const FInteractionOffer& Candidate)
	{
		return Candidate.ActionTag.MatchesTagExact(ActionTag);
	});

	if (!Offer || Offer->RemainingUses < 0)
	{
		return;
	}

	if (Offer->RemainingUses > 0)
	{
		--Offer->RemainingUses;
	}

	const bool bAllExhausted = !Offers.ContainsByPredicate([](const FInteractionOffer& Candidate)
	{
		return Candidate.IsValid() && !Candidate.IsExhausted();
	});

	if (bAllExhausted)
	{
		SetState(EInteractableState::Completed);
	}
	else
	{
		StartCooldown();
	}
}

void UInteractableComponent::StartCooldown()
{
	if (CooldownPeriod <= 0.f)
	{
		return;
	}

	if (!SetState(EInteractableState::Cooldown))
	{
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimer,
			[this]() { SetState(EInteractableState::Idle); },
			CooldownPeriod,
			false);
	}
	
}

void UInteractableComponent::ProcessDependencies()
{
	for (UInteractableComponent* Dependency : Dependencies)
	{
		if (!IsValid(Dependency))
		{
			continue;
		}

		if (State == EInteractableState::Completed)
		{
			Dependency->RemoveSuppression(InteractionTags::Interaction_Suppression_Dependency);
		}
		else
		{
			Dependency->AddSuppression(InteractionTags::Interaction_Suppression_Dependency);
		}
	}
}

void UInteractableComponent::AddSuppression(FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);

	SetState(EInteractableState::Suppressed);
}

void UInteractableComponent::RemoveSuppression(FGameplayTag Reason)
{
	if (!SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.RemoveTag(Reason);

	if (SuppressionReasons.IsEmpty() && State == EInteractableState::Suppressed)
	{
		SetState(EInteractableState::Idle);
	}
}

const FInteractionOffer* UInteractableComponent::FindOffer(const FGameplayTag& InputTag) const
{
	return Offers.FindByPredicate([InputTag](const FInteractionOffer& Offer)
	{
		return Offer.InputTag == InputTag;
	});
}