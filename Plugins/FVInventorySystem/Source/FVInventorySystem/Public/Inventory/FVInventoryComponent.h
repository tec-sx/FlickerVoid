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

	template <typename T>
	const T* FindData() const { return FVFragments::Find<T>(InstanceData); }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ToolTip = "Assigned when the stack enters an inventory; equipment and UI refer to the stack by it."))
	FGuid InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ExcludeBaseStruct, ToolTip = "Stacks with instance data never merge with other stacks."))
	TArray<TInstancedStruct<FFVItemInstanceFragment>> InstanceData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnInventoryItemChanged, UFVItemDefinition*, Item, int32, OldQuantity, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryStackChanged, FGuid, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryItemUsed, UFVItemDefinition*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnInventoryItemDropped, UFVItemDefinition*, Item, int32, Quantity, AActor*, Pickup);
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

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "How many units fit, limited by weight and space."))
	int32 GetAcceptedQuantity(const UFVItemDefinition* Item, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanAddItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetAcceptedQuantity(Item, Quantity) >= Quantity; }

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Returns the quantity actually added (limited by weight and space)."))
	int32 AddItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Adds the whole stack as a new entry, keeping its instance data. Returns its InstanceId, or an invalid id if it didn't fit."))
	FGuid AddStack(FFVItemStack Stack);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Removes nothing and returns false if there isn't enough. Takes from unworn plain stacks first."))
	bool RemoveItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Removes nothing and returns false if the stack holds less, or if dropping a worn container would overflow the inventory."))
	bool RemoveStack(FGuid InstanceId, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "False for quest items, missing stacks or quantities, and worn containers whose loss would overflow the inventory."))
	bool CanDropStack(FGuid InstanceId, int32 Quantity = 1) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Removes units from the stack and spawns them as a pickup in front of the owner; instance data is not carried over. Returns the pickup, or null if nothing was dropped."))
	AActor* DropStack(FGuid InstanceId, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Replaces the instance data of the stack with Stack's InstanceId."))
	bool SetInstanceData(const FFVItemStack& Stack);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Total units across all stacks of the item."))
	int32 GetQuantity(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool HasItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetQuantity(Item) >= Quantity; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool GetStack(FGuid InstanceId, FFVItemStack& OutStack) const;

	const FFVItemStack* FindStack(const FGuid& InstanceId) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	const TArray<FFVItemStack>& GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Stacks whose category matches the tag (children included)."))
	TArray<FFVItemStack> GetItemsInCategory(FGameplayTag Category) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanUseItem(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Applies the item's Usable fragment and consumes one if configured."))
	bool UseItem(UFVItemDefinition* Item);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Base limit from settings plus equipped containers plus WeightBonus."))
	float GetWeightLimit() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetUsedCells() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Pockets plus equipped containers plus CellBonus."))
	int32 GetCellCapacity() const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Extra allowance the game layer grants, e.g. from an Athletics attribute."))
	void SetBonuses(float InWeightBonus, int32 InCellBonus);

	bool FitsAfterChange(float WeightLimitChange, int32 CellCapacityChange, float WeightChange = 0.f, int32 UsedCellChange = 0) const;

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	void SetStackQuantity(const FGuid& InstanceId, int32 NewQuantity);
	bool CanRemoveFromStack(const FFVItemStack& Stack, int32 Quantity) const;
	void BroadcastItemChanged(UFVItemDefinition* Item, int32 OldTotal);
	void RefreshContainers();
	FTransform GetDropTransform() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "Units of an item were added or removed; quantities are totals across stacks."))
	FFVOnInventoryItemChanged OnItemChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "A stack was added, changed or removed (GetStack fails for a removed one)."))
	FFVOnInventoryStackChanged OnStackChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "The contents were replaced wholesale, e.g. by loading a save; rebuild from GetItems."))
	FFVOnInventoryReset OnInventoryReset;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "Weight or space changed, including when a container was equipped or removed."))
	FFVOnInventoryCapacityChanged OnCapacityChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemUsed OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemDropped OnItemDropped;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (TitleProperty = "Item", ToolTip = "Starting contents; also the saved state."))
	TArray<FFVItemStack> Items;

private:
	float ContainerWeightBonus = 0.f;
	int32 ContainerCells = 0;
	float WeightBonus = 0.f;
	int32 CellBonus = 0;
};
