#include "Pickup/FVPickupComponent.h"

#include "GameFramework/Actor.h"
#include "FVInventorySystem.h"
#include "Pickup/FVItemReceiverComponent.h"
#include "Items/FVItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPickupComponent)

bool UFVPickupComponent::CanBePickedUpBy(AActor* Picker) const
{
	const UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(Picker);
	return Item && !IsDepleted() && Receiver && Receiver->IsAcceptingItems();
}

FFVItemStack UFVPickupComponent::GetStack() const
{
	FFVItemStack Stack;
	Stack.Item = Item;
	Stack.Quantity = Quantity;
	Stack.InstanceData = InstanceData;
	return Stack;
}

void UFVPickupComponent::SetStack(const FFVItemStack& Stack)
{
	Item = Stack.Item;
	Quantity = Stack.Quantity;
	InstanceData = Stack.InstanceData;
}

bool UFVPickupComponent::TryPickup(AActor* Picker)
{
	if (!Item)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: pickup has no item."), *GetNameSafe(GetOwner()));
		return false;
	}

	if (IsDepleted())
	{
		return false;
	}

	UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(Picker);
	if (!Receiver)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: %s has no item receiver component."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	const int32 Taken = Receiver->ReceiveStack(GetStack(), GetOwner());
	if (Taken <= 0)
	{
		UE_LOG(LogFVInventorySystem, Verbose, TEXT("%s: %s took nothing, likely out of room."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	// A partial pickup leaves the rest in the world.
	Quantity = FMath::Max(0, Quantity - Taken);
	OnPickedUp.Broadcast(Picker);

	if (IsDepleted() && bDestroyOwnerOnPickup)
	{
		GetOwner()->Destroy();
	}

	return true;
}
