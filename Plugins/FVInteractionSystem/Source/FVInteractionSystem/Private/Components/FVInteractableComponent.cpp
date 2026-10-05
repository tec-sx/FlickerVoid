#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractableResponseComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Conditions/FVConditionStatics.h"
#include "Core/FVInteractionGameplayTags.h"
#include "Data/FVInteractableDefinition.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableComponent)

UFVInteractableComponent::UFVInteractableComponent()
	: State(EFVInteractableState::Idle)
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
EDataValidationResult UFVInteractableComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (!Definition)
	{
		Context.AddError(FText::Format(NSLOCTEXT("FVInteractableComponent", "NoDefinition", "Interactable component '{0}' has no Definition assigned."), FText::FromString(GetName())));
		Result = EDataValidationResult::Invalid;
	}
	return Result;
}
#endif

bool UFVInteractableComponent::ShouldShowOffers() const
{
	return !Definition || Definition->bShowOffers;
}

EFVFocusIndicatorAnchor UFVInteractableComponent::GetFocusIndicatorAnchor() const
{
	return Definition ? Definition->FocusIndicatorAnchor : EFVFocusIndicatorAnchor::Center;
}

FVector UFVInteractableComponent::GetFocusIndicatorOffset() const
{
	return Definition ? Definition->FocusIndicatorOffset : FVector::ZeroVector;
}

FGameplayTag UFVInteractableComponent::GetInteractableType() const
{
	return Definition ? Definition->InteractableType : FGameplayTag(FVInteractionGameplayTags::Interactable);
}

float UFVInteractableComponent::GetCooldownPeriod() const
{
	return Definition ? Definition->CooldownPeriod : 0.f;
}

ECollisionChannel UFVInteractableComponent::GetCollisionChannel() const
{
	return Definition ? Definition->CollisionChannel.GetValue() : UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultCollisionChannel.GetValue();
}

FGameplayTagContainer UFVInteractableComponent::GetCompatibleInteractorTags() const
{
	return Definition ? Definition->CompatibleInteractorTags : FGameplayTagContainer();
}

int32 UFVInteractableComponent::GetDetectionWeight() const
{
	return Definition ? Definition->DetectionWeight : UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultInteractableWeight;
}

void UFVInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(GetOwner()))
	{
		return;
	}

	if (ensureMsgf(Definition, TEXT("%s on %s has no interactable Definition."), *GetName(), *GetNameSafe(GetOwner())))
	{
		RuntimeOffers = Definition->Offers;
	}

	const ECollisionChannel CollisionChannel = GetCollisionChannel();

	TArray<UActorComponent*> DetectableComponents = GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(), DetectablePrimitiveTag);
	
	for (UActorComponent* Component : DetectableComponents)
	{
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetCollisionResponseToChannel(CollisionChannel, ECR_Block);
			DetectablePrimitives.Add(Primitive);
		}
	}

	ProcessDependencies();

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

	if (To == EFVInteractableState::Interacting)
	{
		if (From != EFVInteractableState::Awake && From != EFVInteractableState::Paused)
		{
			LogRejection(TEXT("Only an Awake or Paused interactable can start interacting."));
			return false;
		}
	}
	else if (To == EFVInteractableState::Paused)
	{
		if (From != EFVInteractableState::Interacting)
		{
			LogRejection(TEXT("Only an interacting interactable can be paused."));
			return false;
		}
	}
	else if (To == EFVInteractableState::Cooldown || To == EFVInteractableState::Completed)
	{
		if (From != EFVInteractableState::Interacting && From != EFVInteractableState::Awake)
		{
			LogRejection(TEXT("Only an interacting or awake interactable can finish."));
			return false;
		}
	}

	return true;
}

void UFVInteractableComponent::ActivateInteractions()
{
	if (State != EFVInteractableState::Idle)
		return;

	SetState(SuppressionReasons.IsEmpty() ? EFVInteractableState::Awake : EFVInteractableState::Suppressed);
}

void UFVInteractableComponent::DeactivateInteractions()
{
	if (State == EFVInteractableState::Idle 
		|| State == EFVInteractableState::Completed 
		|| State == EFVInteractableState::Suppressed)
		return;
	
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
	FFVInteractionOffer* Offer = RuntimeOffers.FindByPredicate([&InputTag](const FFVInteractionOffer& Candidate)
	{
		return Candidate.InputTag.MatchesTagExact(InputTag);
	});

	if (!Offer || Offer->RemainingUses <= 0)
	{
		return;
	}

	--Offer->RemainingUses;

	const bool bAllExhausted = !RuntimeOffers.ContainsByPredicate([](const FFVInteractionOffer& Candidate)
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
	const float CooldownPeriod = GetCooldownPeriod();
	if (CooldownPeriod <= 0.f || !SetState(EFVInteractableState::Cooldown))
	{
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			Timer_Cooldown,
			FTimerDelegate::CreateWeakLambda(this, [this]() { SetState(EFVInteractableState::Awake); }),
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
	GetWorld()->GetTimerManager().ClearTimer(Timer_Cooldown);
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

bool UFVInteractableComponent::CanInteract() const
{
	return State == EFVInteractableState::Awake || State == EFVInteractableState::Paused;
}

void UFVInteractableComponent::StartInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor)
{
	SetState(EFVInteractableState::Interacting);
	InteractionStarted.Broadcast(ActionTag, Interactor);
}

void UFVInteractableComponent::ProgressInteraction(
	const FGameplayTag& ActionTag, 
	UFVInteractorComponent* Interactor,
	const float Progress) const
{
	InteractionProgressed.Broadcast(ActionTag, Interactor, Progress);
}

void UFVInteractableComponent::EndInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, const bool bSuccess)
{
	if (State == EFVInteractableState::Interacting)
	{
		SetState(EFVInteractableState::Awake);
	}

	if (bSuccess)
	{
		ApplyOfferEffects(ActionTag, Interactor);
	}

	InteractionEnded.Broadcast(ActionTag, Interactor, bSuccess);
}

void UFVInteractableComponent::ApplyOfferEffects(const FGameplayTag& ActionTag, const UFVInteractorComponent* Interactor) const
{
	const FFVInteractionOffer* Offer = RuntimeOffers.FindByPredicate([&ActionTag](const FFVInteractionOffer& Candidate)
	{
		return Candidate.ActionTag.MatchesTagExact(ActionTag);
	});

	if (Offer && !Offer->Effects.IsEmpty())
	{
		Offer->Effects.Apply(UFVConditionStatics::MakeContext(Interactor ? Interactor->GetOwner() : nullptr, GetOwner()));
	}
}

void UFVInteractableComponent::AcquireInteractor(UFVInteractorComponent* NewInteractor)
{
	if (IsValid(NewInteractor))
	{
		TargetInteractor = NewInteractor;
		InteractorFound.Broadcast(TargetInteractor.Get());
	}
}

void UFVInteractableComponent::ReleaseInteractor(UFVInteractorComponent* InteractorToRelease)
{
	if (UFVInteractorComponent* Interactor = TargetInteractor.Get())
	{
		if (InteractorToRelease == Interactor)
		{
			InteractorLost.Broadcast(Interactor);
			TargetInteractor.Reset();
		}
	}
}

void UFVInteractableComponent::ExecuteAction(const FGameplayTag ActionTag, UFVInteractorComponent* Interactor)
{
	for (UFVInteractableResponseComponent* Response : Responses)
	{
		if (IsValid(Response) && Response->GetActionTag().MatchesTagExact(ActionTag))
		{
			Response->ExecuteAction(Interactor);
		}
	}
}

const FFVInteractionOffer* UFVInteractableComponent::FindOffer(const FGameplayTag& InputTag) const
{
	return RuntimeOffers.FindByPredicate([InputTag](const FFVInteractionOffer& Offer)
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

	Response->SetActionTag(ActionTag);
	Responses.AddUnique(Response);
}

void UFVInteractableComponent::UnbindResponse(UFVInteractableResponseComponent* Response)
{
	if (!IsValid(Response))
	{
		return;
	}

	Responses.RemoveSingleSwap(Response);
	Response->SetActionTag(FGameplayTag());
}
