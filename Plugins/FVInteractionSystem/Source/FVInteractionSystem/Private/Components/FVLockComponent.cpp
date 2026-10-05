#include "Components/FVLockComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLockComponent)

void UFVLockComponent::SetLocked(const bool bInLocked, AActor* Instigator)
{
	if (bLocked == bInLocked)
	{
		return;
	}

	bLocked = bInLocked;
	OnLockStateChanged.Broadcast(bLocked, Instigator);
}
