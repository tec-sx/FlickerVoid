#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FVInventoryComponent.generated.h"

class UFVItemDefinition;

USTRUCT(BlueprintType)
struct FVINVENTORYSYSTEM_API FFVItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ClampMin = 1))
	int32 Quantity = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnInventoryItemChanged, UFVItemDefinition*, Item, int32, OldQuantity, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryItemUsed, UFVItemDefinition*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnInventoryCapacityChanged);

/** Holds items, one entry per item definition. Takes items from a UFVItemReceiverComponent on the same actor. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVINVENTORYSYSTEM_API UFVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInventoryComponent();

	static UFVInventoryComponent* Find(const AActor* Actor);

	/** How many units fit, limited by stack size, weight and space. */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetAcceptedQuantity(const UFVItemDefinition* Item, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanAddItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetAcceptedQuantity(Item, Quantity) >= Quantity; }

	/** Returns the quantity actually added (limited by stack size, weight and space). */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	int32 AddItem(UFVItemDefinition* Item, int32 Quantity = 1);

	/** Removes nothing and returns false if there isn't enough. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool RemoveItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetQuantity(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool HasItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetQuantity(Item) >= Quantity; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	const TArray<FFVItemStack>& GetItems() const { return Items; }

	/** Items whose category matches the tag (children included). */
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

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemChanged OnItemChanged;

	/** Weight or space changed, including when a container was equipped or removed. */
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryCapacityChanged OnCapacityChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemUsed OnItemUsed;

protected:
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

	FFVItemStack* FindStack(const UFVItemDefinition* Item);
	void SetQuantity(UFVItemDefinition* Item, int32 NewQuantity);

	float ContainerWeightBonus = 0.f;
	int32 ContainerCells = 0;
	float WeightBonus = 0.f;
	int32 CellBonus = 0;

	void RefreshContainers();
};
