#include "Objects/FVWorldObjectInteractable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVWorldObjectInteractable)

AFVWorldObjectInteractable::AFVWorldObjectInteractable()
{
}

FGameplayTag AFVWorldObjectInteractable::GetInteractableTag() const
{
	return ObjectTypeTag;
}

void AFVWorldObjectInteractable::OnFocusGained_Implementation(AActor* Interactor)
{ }

void AFVWorldObjectInteractable::OnFocusLost_Implementation(AActor* Interactor)
{ }

FGameplayTagContainer AFVWorldObjectInteractable::GetAvailableActions_Implementation() const
{
	return AvailableActions;
}
