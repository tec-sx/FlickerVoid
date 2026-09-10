#include "Components/ToggleStateResponder.h"

#include "Components/InteractableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ToggleStateResponder)

void UToggleStateResponder::BindInteractableResponses_Implementation(UInteractableComponent* Interactable)
{
	bIsOn = bStartsOn;
	bIsLocked = bStartsLocked;

	Interactable->OnInteractionEnded.AddDynamic(this, &UToggleStateResponder::HandleInteractionEnded);
}

void UToggleStateResponder::SetLocked(bool bLocked)
{
	if (bIsLocked == bLocked)
	{
		return;
	}

	bIsLocked = bLocked;
	OnLockChanged(bIsLocked);
}

void UToggleStateResponder::HandleInteractionEnded(const FGameplayTag& ActionTag, UInteractorComponent* Interactor, bool bSucceeded)
{
	if (!bSucceeded || !ActionTag.MatchesTagExact(ToggleActionTag))
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
