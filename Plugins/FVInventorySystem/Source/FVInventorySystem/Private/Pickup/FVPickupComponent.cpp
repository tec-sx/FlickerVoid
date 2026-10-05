#include "Pickup/FVPickupComponent.h"

#include "GameFramework/Actor.h"
#include "FVInventorySystem.h"
#include "Pickup/FVItemReceiverComponent.h"
#include "Items/FVItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPickupComponent)

bool UFVPickupComponent::CanBePickedUpBy(AActor* Picker) const
{
	const UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(Picker);
	return Item && Receiver && Receiver->IsAcceptingItems();
}

bool UFVPickupComponent::TryPickup(AActor* Picker)
{
	if (!Item)
	{
		UE_LOG(LogFVInventory, Warning, TEXT("%s: pickup has no item."), *GetNameSafe(GetOwner()));
		return false;
	}

	UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(Picker);
	if (!Receiver)
	{
		UE_LOG(LogFVInventory, Warning, TEXT("%s: %s has no item receiver component."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	if (!Receiver->ReceiveItem(Item, Quantity, GetOwner()))
	{
		return false;
	}

	OnPickedUp.Broadcast(Picker);

	if (bDestroyOwnerOnPickup)
	{
		GetOwner()->Destroy();
	}

	return true;
}
