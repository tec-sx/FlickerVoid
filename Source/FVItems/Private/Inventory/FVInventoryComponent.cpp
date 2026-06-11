// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDataAsset.h"
#include "Items/FVItemPickup.h"
#include "Misc/Guid.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryComponent)

UFVInventoryComponent::UFVInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxInventorySlots(20)
	, MaxCarryWeight(100.0f)
{ }

void UFVInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UFVInventoryComponent::AddItem(UFVItemDataAsset* ItemAsset, int32 Quantity)
{
	if (!ItemAsset || Quantity <= 0)
	{
		return false;
	}

	int32 RemainingQuantity = Quantity;

	// Try to add to existing stacks first
	RemainingQuantity = TryAddToExistingStack(ItemAsset, RemainingQuantity);

	// Add remaining quantity as new stacks
	while (RemainingQuantity > 0)
	{
		int32 SlotIndex = FindOrCreateItemSlot(ItemAsset);
		if (SlotIndex == -1)
		{
			// Inventory full
			if (RemainingQuantity < Quantity)
			{
				// Partial add - return success
				OnInventoryChanged.Broadcast(InventoryItems[0], true);
				return true;
			}
			return false;
		}

		int32 QuantityToAdd = FMath::Min(RemainingQuantity, ItemAsset->ItemData.MaxStackSize);
		InventoryItems[SlotIndex].Quantity += QuantityToAdd;
		RemainingQuantity -= QuantityToAdd;

		OnInventoryChanged.Broadcast(InventoryItems[SlotIndex], true);
	}

	OnInventorySlotsChanged.Broadcast(InventoryItems.Num(), MaxInventorySlots);
	return true;
}

int32 UFVInventoryComponent::RemoveItem(const FGuid& InstanceID, int32 Quantity)
{
	int32 RemovedQuantity = 0;

	for (int32 i = InventoryItems.Num() - 1; i >= 0; --i)
	{
		if (InventoryItems[i].InstanceID == InstanceID)
		{
			int32 QuantityToRemove = Quantity > 0 ? FMath::Min(Quantity, InventoryItems[i].Quantity) : InventoryItems[i].Quantity;
			RemovedQuantity = QuantityToRemove;

			InventoryItems[i].Quantity -= QuantityToRemove;

			OnItemRemoved.Broadcast(InventoryItems[i], i, RemovedQuantity);

			if (InventoryItems[i].Quantity <= 0)
			{
				InventoryItems.RemoveAt(i);
			}

			OnInventorySlotsChanged.Broadcast(InventoryItems.Num(), MaxInventorySlots);
			break;
		}
	}

	return RemovedQuantity;
}

//bool UFVInventoryComponent::ExecuteItemAction(const FGuid& InstanceID, FGameplayTag ActionTag)
//{
//	for (FFVItemInstance& Item : InventoryItems)
//	{
//		if (Item.InstanceID == InstanceID)
//		{
//			if (!Item.ItemTemplate)
//			{
//				OnItemActionExecuted.Broadcast(InstanceID, ActionTag, false);
//				return false;
//			}
//
//			const FFVItemDataTemplate& ItemData = Item.ItemTemplate->ItemData;
//
//			// Check if character can use this item
//			if (!IsItemUsable(Item.ItemTemplate))
//			{
//				OnItemActionExecuted.Broadcast(InstanceID, ActionTag, false);
//				return false;
//			}
//
//			// Find the action in available actions
//			const FFVItemAction* FoundAction = ItemData.AvailableActions.FindByPredicate(
//				[&ActionTag](const FFVItemAction& Action)
//				{
//					return Action.ActionTag == ActionTag;
//				}
//			);
//
//			if (!FoundAction)
//			{
//				OnItemActionExecuted.Broadcast(InstanceID, ActionTag, false);
//				return false;
//			}
//
//			AFVCharacter* OwnerCharacter = GetOwnerCharacter();
//			if (!OwnerCharacter)
//			{
//				OnItemActionExecuted.Broadcast(InstanceID, ActionTag, false);
//				return false;
//			}
//
//			// Execute the action
//			Item.ItemTemplate->OnItemUsed(OwnerCharacter);
//
//			// Call blueprint event
//			BP_OnItemActionExecuted(InstanceID, ActionTag);
//
//			// Remove item if action consumes it
//			if (FoundAction->bConsumesItem)
//			{
//				RemoveItem(InstanceID, 1);
//			}
//
//			OnItemActionExecuted.Broadcast(InstanceID, ActionTag, true);
//			return true;
//		}
//	}
//
//	OnItemActionExecuted.Broadcast(InstanceID, ActionTag, false);
//	return false;
//}

void UFVInventoryComponent::InspectItem(const FGuid& InstanceID)
{
	for (const FFVItemInstance& Item : InventoryItems)
	{
		if (Item.InstanceID == InstanceID)
		{
			if (!Item.ItemTemplate)
			{
				return;
			}

			const FFVItemDataTemplate& ItemData = Item.ItemTemplate->ItemData;

			if (!ItemData.bCanInspect)
			{
				return;
			}

			//AFVCharacter* OwnerCharacter = GetOwnerCharacter();
			//if (!OwnerCharacter)
			//{
			//	return;
			//}

			//// Call the blueprint-defined inspect behavior
			//Item.ItemTemplate->OnItemInspected(OwnerCharacter);
			//break;
		}
	}
}

bool UFVInventoryComponent::IsItemUsable(UFVItemDataAsset* ItemAsset) const
{
	if (!ItemAsset)
	{
		return false;
	}

	const FFVItemDataTemplate& ItemData = ItemAsset->ItemData;

	// TODO: Integrate with your Gameplay Ability System
	// For now, we'll use a basic tag check system
	// You'll need to implement GetOwnedGameplayTags() on your character
	
	// Check required tags
	if (ItemData.RequiredTags.Num() > 0)
	{
		// TODO: Check if character has all required tags
		// Example implementation when you have GAS:
		// if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
		// {
		//     if (!ASC->GetOwnedGameplayTags().HasAll(ItemData.RequiredTags))
		//     {
		//         return false;
		//     }
		// }
	}

	// Check blocked tags
	if (ItemData.BlockedTags.Num() > 0)
	{
		// TODO: Check if character has any blocked tags
		// Example implementation when you have GAS:
		// if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
		// {
		//     if (ASC->GetOwnedGameplayTags().HasAny(ItemData.BlockedTags))
		//     {
		//         return false;
		//     }
		// }
	}

	return true;
}

void UFVInventoryComponent::GetItemsByTags(FGameplayTagContainer Tags, bool bExactMatch, TArray<FFVItemInstance>& OutItems)
{
	OutItems.Reset();

	for (const FFVItemInstance& Item : InventoryItems)
	{
		if (Item.ItemTemplate)
		{
			const FGameplayTagContainer& ItemTags = Item.ItemTemplate->ItemData.ItemTags;
			
			if (bExactMatch)
			{
				if (ItemTags.HasAllExact(Tags))
				{
					OutItems.Add(Item);
				}
			}
			else
			{
				if (ItemTags.HasAny(Tags))
				{
					OutItems.Add(Item);
				}
			}
		}
	}
}

void UFVInventoryComponent::GetItemsByTagQuery(FGameplayTag TagQuery, TArray<FFVItemInstance>& OutItems)
{
	OutItems.Reset();

	for (const FFVItemInstance& Item : InventoryItems)
	{
		if (Item.ItemTemplate)
		{
			const FGameplayTagContainer& ItemTags = Item.ItemTemplate->ItemData.ItemTags;
			
			// Check if any item tag matches or is a child of the query tag
			if (ItemTags.HasTag(TagQuery))
			{
				OutItems.Add(Item);
			}
		}
	}
}

bool UFVInventoryComponent::GetItemByInstanceID(const FGuid& InstanceID, FFVItemInstance& OutItem)
{
	for (const FFVItemInstance& Item : InventoryItems)
	{
		if (Item.InstanceID == InstanceID)
		{
			OutItem = Item;
			return true;
		}
	}

	return false;
}

float UFVInventoryComponent::GetTotalWeight() const
{
	float TotalWeight = 0.0f;

	for (const FFVItemInstance& Item : InventoryItems)
	{
		if (Item.ItemTemplate)
		{
			TotalWeight += Item.ItemTemplate->ItemData.Weight * Item.Quantity;
		}
	}

	return TotalWeight;
}

int32 UFVInventoryComponent::GetAvailableSlots() const
{
	return MaxInventorySlots - InventoryItems.Num();
}

int32 UFVInventoryComponent::FindOrCreateItemSlot(UFVItemDataAsset* ItemAsset)
{
	if (!ItemAsset)
	{
		return -1;
	}

	// Check weight limit
	float CurrentWeight = GetTotalWeight();
	float ItemWeight = ItemAsset->ItemData.Weight;

	if (CurrentWeight + ItemWeight > MaxCarryWeight)
	{
		return -1;
	}

	// Check if we have available slots
	if (InventoryItems.Num() >= MaxInventorySlots)
	{
		return -1;
	}

	FFVItemInstance NewItem;
	NewItem.ItemTemplate = ItemAsset;
	NewItem.Quantity = 0;
	NewItem.InstanceID = FGuid::NewGuid();
	NewItem.Durability = ItemAsset->ItemData.bHasDurability ? 1.0f : 0.0f;

	int32 Index = InventoryItems.Add(NewItem);
	return Index;
}

int32 UFVInventoryComponent::TryAddToExistingStack(UFVItemDataAsset* ItemAsset, int32& Quantity)
{
	if (!ItemAsset || ItemAsset->ItemData.MaxStackSize <= 1)
	{
		return Quantity;
	}

	for (FFVItemInstance& Item : InventoryItems)
	{
		if (Item.ItemTemplate == ItemAsset)
		{
			if (Item.Quantity < ItemAsset->ItemData.MaxStackSize)
			{
				int32 SpaceAvailable = ItemAsset->ItemData.MaxStackSize - Item.Quantity;
				int32 QuantityToAdd = FMath::Min(Quantity, SpaceAvailable);

				Item.Quantity += QuantityToAdd;
				Quantity -= QuantityToAdd;

				if (Quantity == 0)
				{
					break;
				}
			}
		}
	}

	return Quantity;
}

//AFVItemPickupActor* UFVInventoryComponent::SpawnItemPickup(UFVItemDataAsset* ItemAsset, int32 Quantity, const FVector& Location)
//{
//	if (!ItemAsset || !PickupActorClass)
//	{
//		return nullptr;
//	}
//
//	UWorld* World = GetWorld();
//	if (!World)
//	{
//		return nullptr;
//	}
//
//	FActorSpawnParameters SpawnParams;
//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
//
//	AFVItemPickupActor* NewPickup = World->SpawnActor<AFVItemPickupActor>(
//		PickupActorClass,
//		Location,
//		FRotator::ZeroRotator,
//		SpawnParams
//	);
//
//	if (NewPickup)
//	{
//		NewPickup->InitializePickup(ItemAsset, Quantity);
//	}
//
//	return NewPickup;
//}
