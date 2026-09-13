#include "Components/FVInteractableResponseComponent_Toggle.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableResponseComponent_Toggle)

void UFVInteractableResponseComponent_Toggle::BindEvents_Implementation(UFVInteractableComponent* Interactable)
{
	bIsOpen = bStartOpen;
	bIsLocked = bStartLocked;
	
	Interactable->InteractionEnded.AddDynamic(this, &UFVInteractableResponseComponent_Toggle::OnInteractionEnded);
}

void UFVInteractableResponseComponent_Toggle::UnbindEvents_Implementation(UFVInteractableComponent* Interactable)
{
	Interactable->InteractionEnded.RemoveDynamic(this, &UFVInteractableResponseComponent_Toggle::OnInteractionEnded);
}

void UFVInteractableResponseComponent_Toggle::SetLocked(const bool bLocked)
{
	bIsLocked = bLocked;
	OnLockChanged(bIsLocked);
}

void UFVInteractableResponseComponent_Toggle::OnInteractionEnded(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, const bool bSuccess)
{
	if (bSuccess && ActionTag.MatchesTagExact(ToggleActionTag))
	{
		if (bIsLocked)
		{
			OnToggleBlocked(!bIsOpen);
			return;
		}

		bIsOpen = !bIsOpen;
		OnToggled(bIsOpen);
	}
}
