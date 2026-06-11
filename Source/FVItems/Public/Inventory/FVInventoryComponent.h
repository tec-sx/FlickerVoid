// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Items/FVItemData.h"
#include "FVInventoryComponent.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UFVItemDataAsset;
class AFVItemPickup;
class AFVCharacter;

/**
 * Delegate for inventory changes
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, const FFVItemInstance&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotsChanged, int32, int32);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemRemoved, const FFVItemInstance&, int32, int32);

/**
 * Delegate for item actions
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemActionExecuted, const FGuid&, ItemInstanceID, FGameplayTag, ActionTag, bool, bSuccess);

/**
 * Component that manages the player's inventory
 * Handles item storage, retrieval, and organization
 */
UCLASS(MinimalAPI, Blueprintable, Meta = (BlueprintSpawnableComponent))
class UFVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInventoryComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/**
	 * Add an item to inventory
	 * @param ItemAsset The item data asset to add
	 * @param Quantity Number of items to add
	 * @return Whether the item was successfully added
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool AddItem(UFVItemDataAsset* ItemAsset, int32 Quantity = 1);

	/**
	 * Remove an item from inventory
	 * @param InstanceID The unique ID of the item instance to remove
	 * @param Quantity Number of items to remove (0 = remove all)
	 * @return Actual quantity removed
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API int32 RemoveItem(const FGuid& InstanceID, int32 Quantity = 1);

	/**
	 * Execute a specific action on an item
	 */
	/*UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool ExecuteItemAction(const FGuid& InstanceID, FGameplayTag ActionTag);*/

	/**
	 * Inspect/examine an item
	 * @param InstanceID The unique ID of the item instance to inspect
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API void InspectItem(const FGuid& InstanceID);

	/**
	 * Check if an item can be used based on required and blocked tags
	 * @param ItemAsset The item to check
	 * @return Whether the item is usable
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool IsItemUsable(UFVItemDataAsset* ItemAsset) const;

	/**
	 * Get all items matching specific tags (for categories/subcategories)
	 * @param Tags Tags to search for
	 * @param bExactMatch If true, item must have ALL tags. If false, ANY tag matches.
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API void GetItemsByTags(FGameplayTagContainer Tags, bool bExactMatch, TArray<FFVItemInstance>& OutItems);

	/**
	 * Get all items matching a single tag query (with parent matching)
	 * E.g., searching for "Item.Category.Weapon" returns all weapon subcategories
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API void GetItemsByTagQuery(FGameplayTag TagQuery, TArray<FFVItemInstance>& OutItems);

	/**
	 * Get an item instance by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool GetItemByInstanceID(const FGuid& InstanceID, FFVItemInstance& OutItem);

	/**
	 * Get total inventory weight
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Inventory")
	UE_API float GetTotalWeight() const;

	/**
	 * Get available inventory slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Inventory")
	UE_API int32 GetAvailableSlots() const;

	/**
	 * Get all items in inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API const TArray<FFVItemInstance>& GetAllItems() const { return InventoryItems; }


	/** Event fired when inventory changes */
	FOnInventoryChanged OnInventoryChanged;

	/** Event fired when inventory slots change */
	FOnInventorySlotsChanged OnInventorySlotsChanged;

	/** Event fired when an item is removed */
	FOnItemRemoved OnItemRemoved;

	/** Event fired when an item action is executed */
	UPROPERTY(BlueprintAssignable, Category = "FlickerVoid|Inventory")
	FOnItemActionExecuted OnItemActionExecuted;

	/** Blueprint event for custom action handling */
	UFUNCTION(BlueprintImplementableEvent, Category = "FlickerVoid|Inventory")
	void BP_OnItemActionExecuted(const FGuid& ItemInstanceID, FGameplayTag ActionTag);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Inventory")
	int32 MaxInventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Inventory", meta = (ClampMin = "0.0"))
	float MaxCarryWeight;

	/** Class to spawn when dropping items */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Inventory")
	TSubclassOf<AFVItemPickup> PickupActorClass;

private:
	UPROPERTY()
	TArray<FFVItemInstance> InventoryItems;

	/**
	 * Find an existing stack for the item or create a new slot
	 */
	int32 FindOrCreateItemSlot(UFVItemDataAsset* ItemAsset);

	/**
	 * Try to add quantity to existing stack
	 */
	int32 TryAddToExistingStack(UFVItemDataAsset* ItemAsset, int32& Quantity);
};

#undef UE_API
