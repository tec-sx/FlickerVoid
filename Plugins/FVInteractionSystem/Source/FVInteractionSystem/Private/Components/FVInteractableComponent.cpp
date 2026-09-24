#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractableResponseComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Core/FVInteractionGameplayTags.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableComponent)

UFVInteractableComponent::UFVInteractableComponent()
	: InteractableType(FVInteractionGameplayTags::Interactable)
	, State(EFVInteractableState::Idle)
	, CooldownPeriod(0.f)
	, CollisionChannel(ECC_Camera)
	, DetectionWeight(1)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	ComponentTags.Add(TEXT("InteractableComponent"));
}

void UFVInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(GetOwner()))
	{
		return;
	}
	
	TArray<UActorComponent*> DetectableComponents = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(), DetectablePrimitiveTag);
	
	for (UActorComponent* Component : DetectableComponents)
	{
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetCollisionResponseToChannel(CollisionChannel, ECR_Block);
			DetectablePrimitives.Add(Primitive);
		}
	}

	Registry = GetWorld()->GetSubsystem<UFVInteractionRegistrySubsystem>();

	if (IsValid(Registry))
	{
		Registry->Register(this);
	}
}

void UFVInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_Cooldown);

		if (IsValid(Registry))
		{
			Registry->Unregister(this);
		}
	}

	Super::EndPlay(EndPlayReason);
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

void UFVInteractableComponent::ActivateInteractions()
{
	if (State == EFVInteractableState::Idle)
	{
		SetState(EFVInteractableState::Awake);
	}
}

void UFVInteractableComponent::DeactivateInteractions()
{
	if (State == EFVInteractableState::Idle)
	{
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_Cooldown);
	}
	
	SetState(EFVInteractableState::Idle);
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

void UFVInteractableComponent::ConsumeOffer(const FGameplayTag& InputTag)
{
	FFVInteractionOffer* Offer = Offers.FindByPredicate([&InputTag](const FFVInteractionOffer& Candidate)
	{
		return Candidate.InputTag.MatchesTagExact(InputTag);
	});

	if (!Offer || Offer->RemainingUses <= 0)
	{
		return;
	}

	--Offer->RemainingUses;

	const bool bAllExhausted = !Offers.ContainsByPredicate([](const FFVInteractionOffer& Candidate)
	{
		return Candidate.IsValid() && !Candidate.IsExhausted();
	});

	if (bAllExhausted)
	{
		SetState(EFVInteractableState::Completed);
		return;
	}

	StartCooldown();
}

void UFVInteractableComponent::StartCooldown()
{
	if (CooldownPeriod <= 0.f || !SetState(EFVInteractableState::Cooldown))
	{
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			Timer_Cooldown,
			[this]() { SetState(EFVInteractableState::Awake); },
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
		EFVInteractableState NewState = EFVInteractableState::Idle;

		if (IsValid(Registry) && Registry->GetActiveInteractables().Contains(this))
		{
			NewState = EFVInteractableState::Awake;
		}

		SetState(NewState);
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
	if (State == EFVInteractableState::Interacting)
	{
		SetState(EFVInteractableState::Awake);
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

void UFVInteractableComponent::BindResponse(FGameplayTag ActionTag, UFVInteractableResponseComponent* Response)
{
	if (!IsValid(Response))
	{
		return;
	}

	if (!ActionTag.IsValid())
	{
		UE_LOG(LogFVInteraction, Error,
			TEXT("'%s' cannot bind response '%s': the supplied ActionTag is invalid."),
			*GetNameSafe(GetOwner()), *Response->GetName());
		return;
	}

	Response->SetBoundActionTag(ActionTag);
	Response->BindEvents(this);
}

void UFVInteractableComponent::UnbindResponse(UFVInteractableResponseComponent* Response)
{
	if (!IsValid(Response))
	{
		return;
	}

	Response->UnbindEvents(this);
	Response->SetBoundActionTag(FGameplayTag());
}
