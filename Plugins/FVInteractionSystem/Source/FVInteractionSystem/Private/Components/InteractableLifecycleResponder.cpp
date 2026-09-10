#include "Components/InteractableLifecycleResponder.h"

#include "Components/InteractableComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableLifecycleResponder)

void UInteractableLifecycleResponder::BindInteractableResponses_Implementation(UInteractableComponent* Interactable)
{
	Interactable->OnInteractionEnded.AddDynamic(this, &UInteractableLifecycleResponder::HandleInteractionEnded);
}

void UInteractableLifecycleResponder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UInteractableLifecycleResponder::HandleInteractionEnded(const FGameplayTag& ActionTag, UInteractorComponent* Interactor, bool bSucceeded)
{
	UInteractableComponent* Interactable = GetInteractable();
	if (!bSucceeded || !Interactable)
	{
		return;
	}

	Interactable->ConsumeOffer(ActionTag);

	if (Interactable->GetState() == EInteractableState::Completed)
	{
		return;
	}

	StartCooldown();
}

void UInteractableLifecycleResponder::StartCooldown()
{
	UInteractableComponent* Interactable = GetInteractable();
	UWorld* World = GetWorld();
	if (!Interactable || !World)
	{
		return;
	}

	if (Interactable->CooldownPeriod <= 0.f)
	{
		return;
	}

	if (!Interactable->TrySetState(EInteractableState::Cooldown))
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		CooldownTimer,
		FTimerDelegate::CreateUObject(this, &UInteractableLifecycleResponder::FinishCooldown),
		Interactable->CooldownPeriod,
		false);
}

void UInteractableLifecycleResponder::FinishCooldown()
{
	if (UInteractableComponent* Interactable = GetInteractable())
	{
		Interactable->TrySetState(EInteractableState::Idle);
	}
}
