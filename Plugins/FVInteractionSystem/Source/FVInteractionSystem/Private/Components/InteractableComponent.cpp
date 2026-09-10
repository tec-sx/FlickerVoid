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

	UInteractionRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>();

	if (Registry)
	{
		Registry->Register(this);
	}
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UInteractionRegistrySubsystem* Registry = GetWorld() 
		? GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>() 
		: nullptr;

	if (Registry)
	{
		Registry->Unregister(this);
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
	OnFocusStateChanged.Broadcast(bIsInFocus, Interactor);
}

namespace
{
	void LogRejectedTransition(EInteractableState From, EInteractableState To, const TCHAR* Reason)
	{
		UE_LOG(LogFVInteraction, Verbose, TEXT("Rejected %s -> %s: %s"),
			*UEnum::GetValueAsString(From),
			*UEnum::GetValueAsString(To),
			Reason);
	}
}

bool UInteractableComponent::IsTransitionAllowed(EInteractableState From, EInteractableState To)
{
	if (To == EInteractableState::Default)
	{
		LogRejectedTransition(From, To, TEXT("Default is not a valid target state."));
		return false;
	}

	if (From == To)
	{
		LogRejectedTransition(From, To, TEXT("Already in the requested state."));
		return false;
	}

	if (From == EInteractableState::Completed)
	{
		LogRejectedTransition(From, To, TEXT("Completed is terminal."));
		return false;
	}

	switch (To)
	{
	case EInteractableState::Awake:
		if (From == EInteractableState::Interacting)
		{
			LogRejectedTransition(From, To, TEXT("Finish or cancel the interaction before waking."));
			return false;
		}
		break;

	case EInteractableState::Interacting:
		if (From != EInteractableState::Awake && From != EInteractableState::Paused)
		{
			LogRejectedTransition(From, To, TEXT("Only an Awake or Paused interactable can start interacting."));
			return false;
		}
		break;

	case EInteractableState::Paused:
		if (From != EInteractableState::Interacting)
		{
			LogRejectedTransition(From, To, TEXT("Only an interacting interactable can be paused."));
			return false;
		}
		break;

	case EInteractableState::Cooldown:
	case EInteractableState::Completed:
		if (From != EInteractableState::Interacting && From != EInteractableState::Awake)
		{
			LogRejectedTransition(From, To, TEXT("Only an interacting or awake interactable can finish."));
			return false;
		}
		break;

	default:
		break;
	}

	return true;
}

bool UInteractableComponent::TrySetState(EInteractableState NewState)
{
	if (!IsTransitionAllowed(State, NewState))
	{
		return false;
	}

	const EInteractableState OldState = State;
	State = NewState;

	ApplyStateTag(OldState, NewState);
	OnStateChanged.Broadcast(NewState);
	ProcessDependencies();

	return true;
}

void UInteractableComponent::ApplyStateTag(EInteractableState OldState, EInteractableState NewState)
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
		TrySetState(EInteractableState::Completed);
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

	TrySetState(EInteractableState::Suppressed);
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
		TrySetState(EInteractableState::Idle);
	}
}

const FInteractionOffer* UInteractableComponent::FindOffer(const FGameplayTag& InputTag) const
{
	return Offers.FindByPredicate([InputTag](const FInteractionOffer& Offer)
	{
		return Offer.InputTag == InputTag;
	});
}