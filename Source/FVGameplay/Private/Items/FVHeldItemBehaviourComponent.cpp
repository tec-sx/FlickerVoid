#include "Items/FVHeldItemBehaviourComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVHeldItemBehaviourComponent)

void UFVHeldItemBehaviourComponent::Use(AActor* User)
{
	if (bToggleState)
	{
		bOn = !bOn;
		OnToggled.Broadcast(bOn);
	}

	OnUsed.Broadcast(User);
}
