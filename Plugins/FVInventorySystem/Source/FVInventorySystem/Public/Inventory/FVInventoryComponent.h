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

	template <typename T>
	T& FindOrAddData()
	{
		for (TInstancedStruct<FFVItemInstanceFragment>& Data : InstanceData)
		{
			if (T* Found = Data.GetMutablePtr<T>())
			{
				return *Found;
			}
		}

		TInstancedStruct<FFVItemInstanceFragment>& Added = InstanceData.AddDefaulted_GetRef();
		Added.InitializeAs<T>();
		return *Added.GetMutablePtr<T>();
	}

	template <typename T>
	bool RemoveData()
	{
		return InstanceData.RemoveAll([](const TInstancedStruct<FFVItemInstanceFragment>& Data) { return Data.GetPtr<T>() != nullptr; }) > 0;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ToolTip = "Assigned when the stack enters an inventory; equipment and UI refer to the stack by it."))
	FGuid InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Inventory")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Inventory", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ExcludeBaseStruct, ToolTip = "Stacks with instance data never merge with other stacks."))
	TArray<TInstancedStruct<FFVItemInstanceFragment>> InstanceData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnInventoryItemChanged, UFVItemDefinition*, Item, int32, OldQuantity, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryStackChanged, FGuid, InstanceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryItemUsed, UFVItemDefinition*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnInventoryMoneyChanged, int32, OldMoney, int32, NewMoney);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnInventoryCapacityChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnInventoryReset);

/**
 * Holds item stacks and money. Units of an item without instance data merge up to the item's MaxQuantity per stack;
 * each stack claims the item's footprint in cells. Key items (settings Key Item Categories) take no weight or cells.
 * Takes items from a UFVItemReceiverComponent on the same actor.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVINVENTORYSYSTEM_API UFVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInventoryComponent();

	static UFVInventoryComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "How many units fit, limited by weight and space. Key items and inventories without carry limits take any amount."))
	int32 GetAcceptedQuantity(const UFVItemDefinition* Item, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanAddItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetAcceptedQuantity(Item, Quantity) >= Quantity; }

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Returns the quantity actually added (limited by weight and space)."))
	int32 AddItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Use for items the story grants (quest rewards, story items): adds every unit, ignoring weight and space, so they are never lost. Returns the quantity added."))
	int32 AddStoryItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Adds the whole stack as a new entry, keeping its instance data. Returns its InstanceId, or an invalid id if it didn't fit."))
	FGuid AddStack(FFVItemStack Stack);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Removes nothing and returns false if there isn't enough. Takes from unworn plain stacks first."))
	bool RemoveItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Removes nothing and returns false if the stack holds less, or if dropping a worn container would overflow the inventory."))
	bool RemoveStack(FGuid InstanceId, int32 Quantity = 1);

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

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Stacks of everything but key items: the storage list."))
	TArray<FFVItemStack> GetStorageItems() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Stacks of key items: the Keys and Quest list."))
	TArray<FFVItemStack> GetKeyItems() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "First stack whose item definition Tags contain the tag or a child of it, e.g. a key for a lock or a lockpick."))
	bool FindItemWithTag(FGameplayTag Tag, FFVItemStack& OutStack) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Total units of items whose definition Tags contain the tag or a child of it."))
	int32 GetQuantityWithTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "A stack of the item can be used: see Can Use Stack."))
	bool CanUseItem(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Uses the item from one of its stacks: a partly used unit first, else an unworn plain stack. See Use Stack."))
	bool UseItem(UFVItemDefinition* Item);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "The stack's item is usable and its Use Conditions pass for the owner."))
	bool CanUseStack(FGuid InstanceId) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Applies the item's Usable effects to the owner. With Charges one use is spent (a partly used unit becomes its own stack) and the unit goes after its last use; otherwise one unit is consumed if Consume On Use."))
	bool UseStack(FGuid InstanceId);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Uses left on the stack's next unit; 0 if the stack doesn't exist."))
	int32 GetCharges(FGuid InstanceId) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Moves up to Quantity units of an unworn stack to another inventory, keeping instance data (a stack with instance data moves whole or not at all). Returns the units moved."))
	int32 TransferStack(UFVInventoryComponent* To, FGuid InstanceId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Like Transfer Stack, but the moved units get the Stolen mark with StolenFrom as owner (empty = unknown owner)."))
	int32 TransferStackAsStolen(UFVInventoryComponent* To, FGuid InstanceId, int32 Quantity, FGameplayTag StolenFrom);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Moves every unworn stack, and the money if asked, to another inventory. A valid StolenFrom marks the moved items stolen from that owner. Returns the number of stacks moved whole."))
	int32 TransferAll(UFVInventoryComponent* To, bool bIncludeMoney = true, FGameplayTag StolenFrom = FGameplayTag());

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Money is a number per character, not an item."))
	int32 GetMoney() const { return Money; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Money is at least Amount."))
	bool CanAfford(int32 Amount) const { return Money >= Amount; }

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Ignores amounts of 0 or less."))
	void AddMoney(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Removes nothing and returns false if there isn't enough."))
	bool RemoveMoney(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Moves Amount, or all there is if less, to another inventory. Returns the amount moved."))
	int32 TransferMoney(UFVInventoryComponent* To, int32 Amount);

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
	void HandleStackReceived(const FFVItemStack& Stack, AActor* Source);

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
	bool CanAcceptStack(const FFVItemStack& Stack) const;
	int32 AddStackInParts(FFVItemStack Stack);
	int32 AddUnits(UFVItemDefinition* Item, int32 Added);
	FGuid FindStackToUse(const UFVItemDefinition* Item) const;
	void SpendUse(const FGuid& InstanceId);
	int32 MoveStack(UFVInventoryComponent* To, const FGuid& InstanceId, int32 Quantity, bool bMarkStolen, const FGameplayTag& StolenFrom);
	void BroadcastItemChanged(UFVItemDefinition* Item, int32 OldTotal);
	void RefreshContainers();

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

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "Money changed; also broadcast with equal values after a load."))
	FFVOnInventoryMoneyChanged OnMoneyChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (TitleProperty = "Item", ToolTip = "Starting contents; also the saved state."))
	TArray<FFVItemStack> Items;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ClampMin = 0, ToolTip = "Starting money; also the saved state."))
	int32 Money = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ToolTip = "Weight and space limits apply. Off for loot containers, merchants' stock and other storage."))
	bool bApplyCarryLimits = true;

private:
	float ContainerWeightBonus = 0.f;
	int32 ContainerCells = 0;
	float WeightBonus = 0.f;
	int32 CellBonus = 0;
};
