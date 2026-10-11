#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/FVFragment.h"
#include "GameplayTagContainer.h"
#include "FVInventoryComponent.generated.h"

class UFVItemDefinition;

/** Base for per-instance item data (dye, wear, a stolen flag...). Mark fields SaveGame so they are saved. */
USTRUCT(BlueprintType, meta = (Hidden))
struct FVINVENTORYSYSTEM_API FFVItemInstanceFragment : public FFVFragment
{
	GENERATED_BODY()
};

/** One inventory entry: some units of an item, optionally carrying instance data. */
USTRUCT(BlueprintType)
struct FVINVENTORYSYSTEM_API FFVItemStack
{
	GENERATED_BODY()

	/** Assigned when the stack enters an inventory; equipment and UI refer to the stack by it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	FGuid InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ClampMin = 1))
	int32 Quantity = 1;

	/** Stacks with instance data never merge with other stacks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVItemInstanceFragment>> InstanceData;

	template <typename T>
	const T* FindData() const { return FVFragments::Find<T>(InstanceData); }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnInventoryItemChanged, UFVItemDefinition*, Item, int32, OldQuantity, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryStackChanged, FGuid, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryItemUsed, UFVItemDefinition*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnInventoryCapacityChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnInventoryReset);

/**
 * Holds item stacks. Units of an item without instance data merge up to the item's MaxQuantity per stack; each stack
 * claims the item's footprint in cells. Takes items from a UFVItemReceiverComponent on the same actor.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVINVENTORYSYSTEM_API UFVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInventoryComponent();

	static UFVInventoryComponent* Find(const AActor* Actor);

	/** How many units fit, limited by weight and space. */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetAcceptedQuantity(const UFVItemDefinition* Item, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanAddItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetAcceptedQuantity(Item, Quantity) >= Quantity; }

	/** Returns the quantity actually added (limited by weight and space). */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	int32 AddItem(UFVItemDefinition* Item, int32 Quantity = 1);

	/** Adds the whole stack as a new entry, keeping its instance data. Returns its InstanceId, or an invalid id if it didn't fit. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	FGuid AddStack(FFVItemStack Stack);

	/** Removes nothing and returns false if there isn't enough. Takes from unequipped plain stacks first. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool RemoveItem(UFVItemDefinition* Item, int32 Quantity = 1);

	/** Removes nothing and returns false if the stack holds less, or if dropping a worn container would overflow the inventory. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool RemoveStack(FGuid InstanceId, int32 Quantity = 1);

	/** Replaces the instance data of the stack with Stack's InstanceId. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool SetInstanceData(const FFVItemStack& Stack);

	/** Total units across all stacks of the item. */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetQuantity(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool HasItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetQuantity(Item) >= Quantity; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool GetStack(FGuid InstanceId, FFVItemStack& OutStack) const;

	const FFVItemStack* FindStack(const FGuid& InstanceId) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	const TArray<FFVItemStack>& GetItems() const { return Items; }

	/** Stacks whose category matches the tag (children included). */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	TArray<FFVItemStack> GetItemsInCategory(FGameplayTag Category) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanUseItem(const UFVItemDefinition* Item) const;

	/** Applies the item's Usable fragment and consumes one if configured. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool UseItem(UFVItemDefinition* Item);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	float GetTotalWeight() const;

	/** Base limit from settings plus equipped containers plus WeightBonus. */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	float GetWeightLimit() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetUsedCells() const;

	/** Pockets plus equipped containers plus CellBonus. */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetCellCapacity() const;

	/** Extra allowance the game layer grants, e.g. from an Athletics attribute. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	void SetBonuses(float InWeightBonus, int32 InCellBonus);

	/**
	 * The overflow rule: capacity may only shrink (a container taken off, swapped or dropped) if what is carried still
	 * fits afterwards. Limits that are switched off always fit; a change that doesn't shrink capacity is always allowed.
	 */
	bool FitsAfterChange(float WeightLimitChange, int32 CellCapacityChange, float WeightChange = 0.f, int32 UsedCellChange = 0) const;

	/** Units of an item were added or removed; quantities are totals across stacks. */
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemChanged OnItemChanged;

	/** A stack was added, changed or removed (GetStack fails for a removed one). */
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryStackChanged OnStackChanged;

	/** The contents were replaced wholesale, e.g. by loading a save; rebuild from GetItems. */
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryReset OnInventoryReset;

	/** Weight or space changed, including when a container was equipped or removed. */
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryCapacityChanged OnCapacityChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemUsed OnItemUsed;

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Starting contents; also the saved state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (TitleProperty = "Item"))
	TArray<FFVItemStack> Items;

private:
	UFUNCTION()
	void HandleItemReceived(UFVItemDefinition* Item, int32 Quantity, AActor* Source);

	UFUNCTION()
	void HandleActorDataLoaded();

	UFUNCTION()
	void HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem);

	UFUNCTION()
	void HandleEquipmentReset();

	FFVItemStack* FindMutableStack(const FGuid& InstanceId);
	void AssignMissingIds();

	/** Sets a stack's quantity, removing it at 0, and broadcasts. */
	void SetStackQuantity(const FGuid& InstanceId, int32 NewQuantity);
	bool CanRemoveFromStack(const FFVItemStack& Stack, int32 Quantity) const;
	void BroadcastItemChanged(UFVItemDefinition* Item, int32 OldTotal);

	float ContainerWeightBonus = 0.f;
	int32 ContainerCells = 0;
	float WeightBonus = 0.f;
	int32 CellBonus = 0;

	void RefreshContainers();
};
