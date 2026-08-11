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

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool AddItem(UFVItemDataAsset* ItemAsset, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API int32 RemoveItem(const FGuid& InstanceID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API void InspectItem(const FGuid& InstanceID);

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool IsItemUsable(UFVItemDataAsset* ItemAsset) const;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API void GetItemsByTags(FGameplayTagContainer Tags, bool bExactMatch, TArray<FFVItemInstance>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API void GetItemsByTagQuery(FGameplayTag TagQuery, TArray<FFVItemInstance>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API bool GetItemByInstanceID(const FGuid& InstanceID, FFVItemInstance& OutItem);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Inventory")
	UE_API float GetTotalWeight() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Inventory")
	UE_API int32 GetAvailableSlots() const;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Inventory")
	UE_API const TArray<FFVItemInstance>& GetAllItems() const { return InventoryItems; }

	FOnInventoryChanged OnInventoryChanged;
	FOnInventorySlotsChanged OnInventorySlotsChanged;
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "FlickerVoid|Inventory")
	FOnItemActionExecuted OnItemActionExecuted;

	UFUNCTION(BlueprintImplementableEvent, Category = "FlickerVoid|Inventory")
	void BP_OnItemActionExecuted(const FGuid& ItemInstanceID, FGameplayTag ActionTag);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Inventory")
	int32 MaxInventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Inventory", meta = (ClampMin = "0.0"))
	float MaxCarryWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Inventory")
	TSubclassOf<AFVItemPickup> PickupActorClass;

private:
	UPROPERTY()
	TArray<FFVItemInstance> InventoryItems;
	int32 FindOrCreateItemSlot(UFVItemDataAsset* ItemAsset);
	int32 TryAddToExistingStack(UFVItemDataAsset* ItemAsset, int32& Quantity);

	void BroadcastInventoryChanged(const FFVItemInstance& Item);
	void BroadcastInventoryChangedRaw(const FGuid& InstanceID, int32 NewQuantity);
};

#undef UE_API
