#include "Components/InteractableToggleComponent.h"

#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableToggleComponent)

void UInteractableToggleComponent::BeginPlay()
{
	Super::BeginPlay();
	
	bIsOn = bInitialState;
	bIsLocked = bStartsLocked;
}

void UInteractableToggleComponent::BindInteractions_Implementation(UInteractorComponent* Interactor)
{
	Interactor->InteractionCommited.AddDynamic(this, &UInteractableToggleComponent::HandleInteractionEnded);
}

void UInteractableToggleComponent::UnbindInteractions_Implementation(UInteractorComponent* Interactor)
{
	Interactor->InteractionCommited.RemoveDynamic(this, &UInteractableToggleComponent::HandleInteractionEnded);
}

void UInteractableToggleComponent::SetLocked(const bool bLocked)
{
	bIsLocked = bLocked;
	OnLockChanged(bIsLocked);
}

void UInteractableToggleComponent::HandleInteractionEnded(const FInteractionCommit& Commit, bool bSuccess)
{
	if (!bSuccess || !Commit.ActionTag.MatchesTagExact(ToggleActionTag))
	{
		return;
	}

	if (bIsLocked)
	{
		OnToggleBlocked(!bIsOn);
		return;
	}

	bIsOn = !bIsOn;
	OnToggled(bIsOn);
}
