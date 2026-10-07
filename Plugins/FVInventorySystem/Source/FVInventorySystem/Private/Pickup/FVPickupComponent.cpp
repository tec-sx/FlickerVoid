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
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: pickup has no item."), *GetNameSafe(GetOwner()));
		return false;
	}

	UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(Picker);
	if (!Receiver)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: %s has no item receiver component."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	const int32 Taken = Receiver->ReceiveItem(Item, Quantity, GetOwner());
	if (Taken <= 0)
	{
		UE_LOG(LogFVInventorySystem, Verbose, TEXT("%s: %s took nothing, likely out of room."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	OnPickedUp.Broadcast(Picker);

	// A partial pickup leaves the rest in the world.
	if (Taken < Quantity)
	{
		Quantity -= Taken;
		return true;
	}

	if (bDestroyOwnerOnPickup)
	{
		GetOwner()->Destroy();
	}

	return true;
}
