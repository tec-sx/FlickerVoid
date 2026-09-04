#include "Core/InteractionRequirement.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionRequirement)

bool UInteractionRequirement::IsMet_Implementation(
	const FGameplayTag ActionTag, 
	const UInteractorComponent* Interactor, 
	const UInteractableComponent* Interactable) const
{
	return true;
}
