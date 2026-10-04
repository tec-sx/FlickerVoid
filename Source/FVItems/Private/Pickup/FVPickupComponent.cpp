#include "Pickup/FVPickupComponent.h"

#include "GameFramework/Actor.h"
#include "Interfaces/FVItemReceiver.h"
#include "Items/FVItemDataAsset.h"
#include "Logging/FVLogCategories.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPickupComponent)

bool UFVPickupComponent::CanBePickedUpBy(AActor* Picker) const
{
	const UObject* Receiver = IFVItemReceiver::FindReceiver(Picker);
	return Item && Receiver && IFVItemReceiver::Execute_CanReceiveItem(Receiver, Item, Quantity);
}

bool UFVPickupComponent::TryPickup(AActor* Picker)
{
	if (!Item)
	{
		UE_LOG(LogFVItems, Warning, TEXT("%s: pickup has no item."), *GetNameSafe(GetOwner()));
		return false;
	}

	UObject* Receiver = IFVItemReceiver::FindReceiver(Picker);
	if (!Receiver)
	{
		UE_LOG(LogFVItems, Warning, TEXT("%s: %s has nothing that can receive items."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	if (!IFVItemReceiver::Execute_CanReceiveItem(Receiver, Item, Quantity)
		|| !IFVItemReceiver::Execute_ReceiveItem(Receiver, Item, Quantity))
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
