#include "Pickup/FVPickupComponent.h"

#include "Definitions/FVInventoryItemTemplate.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Logging/FVLogCategories.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPickupComponent)

namespace
{
	UObject* FindInventoryOn(const AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (Actor->Implements<UFVAdvancedInventoryInterface>())
		{
			return const_cast<AActor*>(Actor);
		}

		TArray<UActorComponent*> Inventories = Actor->GetComponentsByInterface(UFVAdvancedInventoryInterface::StaticClass());
		return Inventories.IsEmpty() ? nullptr : Inventories[0];
	}
}

UObject* UFVPickupComponent::FindInventory(AActor* Actor)
{
	if (UObject* Inventory = FindInventoryOn(Actor))
	{
		return Inventory;
	}

	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (UObject* Inventory = FindInventoryOn(Pawn->GetController()))
		{
			return Inventory;
		}

		return FindInventoryOn(Pawn->GetPlayerState());
	}

	return nullptr;
}

bool UFVPickupComponent::CanBePickedUpBy(AActor* Picker) const
{
	UObject* Inventory = FindInventory(Picker);
	return ItemTemplate && Inventory
		&& IFVAdvancedInventoryInterface::Execute_CanAddItemFromTemplate(Inventory, ItemTemplate, Quantity);
}

bool UFVPickupComponent::TryPickup(AActor* Picker)
{
	if (!ItemTemplate)
	{
		UE_LOG(LogFVInventory, Warning, TEXT("%s: pickup has no item template."), *GetNameSafe(GetOwner()));
		return false;
	}

	UObject* Inventory = FindInventory(Picker);
	if (!Inventory)
	{
		UE_LOG(LogFVInventory, Warning, TEXT("%s: no inventory found on %s."), *GetNameSafe(GetOwner()), *GetNameSafe(Picker));
		return false;
	}

	if (!IFVAdvancedInventoryInterface::Execute_CanAddItemFromTemplate(Inventory, ItemTemplate, Quantity)
		|| !IFVAdvancedInventoryInterface::Execute_AddItemFromTemplate(Inventory, ItemTemplate, Quantity, Durability))
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
