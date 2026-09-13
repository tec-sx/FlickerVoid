#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Core/FVInteractionGameplayTags.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableComponent)

UFVInteractableComponent::UFVInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFVInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(GetOwner()))
	{
		return;
	}

	const FFVInteractableSettings& Defaults = UFVInteractionSystemSettings::Get().InteractableBaseSettings;

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

	if (UFVInteractionRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UFVInteractionRegistrySubsystem>())
	{
		Registry->Register(this);
	}
}

void UFVInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimer);

		if (UFVInteractionRegistrySubsystem* Registry = World->GetSubsystem<UFVInteractionRegistrySubsystem>())
		{
			Registry->Unregister(this);
		}
	}
	
	bIsInitialized = false;

	Super::EndPlay(EndPlayReason);
}

FVector UFVInteractableComponent::GetFocusPoint() const
{
	if (const UPrimitiveComponent* Primitive = FocusPrimitive.Get())
	{
		return Primitive->Bounds.Origin;
	}

	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

void UFVInteractableComponent::SetFocused(bool bFocused, UFVInteractorComponent* Interactor)
{
	if (!bIsInitialized || bFocused == bIsInFocus)
	{
		return;
	}

	bIsInFocus = bFocused;
}

bool UFVInteractableComponent::IsTransitionAllowed(EFVInteractableState From, EFVInteractableState To)
{
	auto LogRejection = [&From, &To](const TCHAR* Reason)
	{
		UE_LOG(LogFVInteraction, Verbose, TEXT("Rejected %s -> %s: %s"),
			*UEnum::GetValueAsString(From),
			*UEnum::GetValueAsString(To),
			Reason);
	};
	
	if (To == EFVInteractableState::Default)
	{
		LogRejection(TEXT("Default is not a valid target state."));
		return false;
	}

	if (From == To)
	{
		LogRejection(TEXT("Already in the requested state."));
		return false;
	}

	if (From == EFVInteractableState::Completed)
	{
		LogRejection(TEXT("Completed is terminal."));
		return false;
	}

	switch (To)
	{
	case EFVInteractableState::Awake:
		if (From == EFVInteractableState::Interacting)
		{
			LogRejection(TEXT("Finish or cancel the interaction before waking."));
			return false;
		}
		break;

	case EFVInteractableState::Interacting:
		if (From != EFVInteractableState::Awake && From != EFVInteractableState::Paused)
		{
			LogRejection(TEXT("Only an Awake or Paused interactable can start interacting."));
			return false;
		}
		break;

	case EFVInteractableState::Paused:
		if (From != EFVInteractableState::Interacting)
		{
			LogRejection(TEXT("Only an interacting interactable can be paused."));
			return false;
		}
		break;

	case EFVInteractableState::Cooldown:
	case EFVInteractableState::Completed:
		if (From != EFVInteractableState::Interacting && From != EFVInteractableState::Awake)
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

bool UFVInteractableComponent::SetState(EFVInteractableState NewState)
{
	if (!IsTransitionAllowed(State, NewState))
	{
		return false;
	}

	const EFVInteractableState OldState = State;
	State = NewState;

	ApplyStateTag(OldState, NewState);
	StateChanged.Broadcast(NewState);
	ProcessDependencies();

	return true;
}

void UFVInteractableComponent::ApplyStateTag(const EFVInteractableState OldState, const EFVInteractableState NewState) const
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	const FGameplayTag OldTag = FVInteractionGameplayTags::StateToTag(OldState);
	const FGameplayTag NewTag = FVInteractionGameplayTags::StateToTag(NewState);

	if (OldTag.IsValid())
	{
		OwningActor->Tags.Remove(OldTag.GetTagName());
	}

	if (NewTag.IsValid())
	{
		OwningActor->Tags.AddUnique(NewTag.GetTagName());
	}
}

void UFVInteractableComponent::ConsumeOffer(const FGameplayTag& ActionTag)
{
	FFVInteractionOffer* Offer = Offers.FindByPredicate([&ActionTag](const FFVInteractionOffer& Candidate)
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

	const bool bAllExhausted = !Offers.ContainsByPredicate([](const FFVInteractionOffer& Candidate)
	{
		return Candidate.IsValid() && !Candidate.IsExhausted();
	});

	if (bAllExhausted)
	{
		SetState(EFVInteractableState::Completed);
	}
	else
	{
		StartCooldown();
	}
}

void UFVInteractableComponent::StartCooldown()
{
	if (CooldownPeriod <= 0.f)
	{
		return;
	}

	if (!SetState(EFVInteractableState::Cooldown))
	{
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimer,
			[this]() { SetState(EFVInteractableState::Idle); },
			CooldownPeriod,
			false);
	}
	
}

void UFVInteractableComponent::ProcessDependencies()
{
	for (UFVInteractableComponent* Dependency : Dependencies)
	{
		if (!IsValid(Dependency))
		{
			continue;
		}

		if (State == EFVInteractableState::Completed)
		{
			Dependency->RemoveSuppression(FVInteractionGameplayTags::Interaction_Suppression_Dependency);
		}
		else
		{
			Dependency->AddSuppression(FVInteractionGameplayTags::Interaction_Suppression_Dependency);
		}
	}
}

void UFVInteractableComponent::AddSuppression(FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);

	SetState(EFVInteractableState::Suppressed);
}

void UFVInteractableComponent::RemoveSuppression(FGameplayTag Reason)
{
	if (!SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.RemoveTag(Reason);

	if (SuppressionReasons.IsEmpty() && State == EFVInteractableState::Suppressed)
	{
		SetState(EFVInteractableState::Idle);
	}
}

void UFVInteractableComponent::StartInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor)
{
	SetState(EFVInteractableState::Interacting);
	InteractionStarted.Broadcast(ActionTag, Interactor);
}

void UFVInteractableComponent::ProgressInteraction(
	const FGameplayTag& ActionTag, 
	UFVInteractorComponent* Interactor,
	float Progress)
{
	InteractionProgressed.Broadcast(ActionTag, Interactor, Progress);
}

void UFVInteractableComponent::EndInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, const bool bSuccess)
{
	SetState(EFVInteractableState::Awake);
	
	if (bSuccess)
	{
		ConsumeOffer(ActionTag);
	}
	
	InteractionEnded.Broadcast(ActionTag, Interactor, bSuccess);
}

const FFVInteractionOffer* UFVInteractableComponent::FindOffer(const FGameplayTag& InputTag) const
{
	return Offers.FindByPredicate([InputTag](const FFVInteractionOffer& Offer)
	{
		return Offer.InputTag == InputTag;
	});
}
