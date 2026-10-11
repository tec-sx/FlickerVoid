#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FVEquipmentComponent.generated.h"

class UAbilitySystemComponent;
class UFVInventoryComponent;
class UFVItemDefinition;

USTRUCT(BlueprintType)
struct FVINVENTORYSYSTEM_API FFVEquippedItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag Slot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Equipment")
	TObjectPtr<UFVItemDefinition> Item;

	/** The worn inventory stack; invalid for items worn without being carried (Require Inventory off). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Equipment")
	FGuid InstanceId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnEquipmentChanged, FGameplayTag, Slot, UFVItemDefinition*, OldItem, UFVItemDefinition*, NewItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnEquipmentReset);

/**
 * Wears equippable items (outfits, accessories), one per slot. An item goes into one of its Slots and covers its
 * Blocked Slots; whatever conflicts is taken off. Granted tags of worn items are added to the owner's ability system
 * as loose tags, so conditions and AI can read disguises and dress codes.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVINVENTORYSYSTEM_API UFVEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVEquipmentComponent();

	static UFVEquipmentComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	bool CanEquip(const UFVItemDefinition* Item) const;

	/** Wears a carried, unworn stack of the item in its first free slot, taking off whatever conflicts. */
	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	bool Equip(UFVItemDefinition* Item);

	/** Slot None picks the first of the item's slots that is free. */
	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	bool CanEquipInstance(FGuid InstanceId, FGameplayTag Slot) const;

	/** Wears a specific inventory stack. Slot None picks the first of the item's slots that is free. */
	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	bool EquipInstance(FGuid InstanceId, FGameplayTag Slot);

	/** False if the slot is empty, or if taking off a container would leave more carried than fits. */
	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	bool CanUnequip(FGameplayTag Slot) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	bool Unequip(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	UFVItemDefinition* GetEquipped(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	FGuid GetEquippedInstance(FGameplayTag Slot) const;

	/** The worn item whose Blocked Slots cover this slot, if any. */
	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	UFVItemDefinition* GetBlockingItem(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	bool IsEquipped(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	bool IsInstanceEquipped(FGuid InstanceId) const;

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	const TArray<FFVEquippedItem>& GetAllEquipped() const { return Equipped; }

	/** Union of the granted tags of everything worn. */
	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	FGameplayTagContainer GetGrantedTags() const;

	/** Applies the difference between granted and applied tags; call when the owner's ability system becomes available after BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	void RefreshGrantedTags();

	UPROPERTY(BlueprintAssignable, Category = "FV|Equipment")
	FFVOnEquipmentChanged OnEquipmentChanged;

	/** All slots were replaced, e.g. by loading a save; rebuild from GetAllEquipped. */
	UPROPERTY(BlueprintAssignable, Category = "FV|Equipment")
	FFVOnEquipmentReset OnEquipmentReset;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** If set, only items held in the owner's inventory can be equipped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	bool bRequireInventory = true;

	/** Starting equipment; also the saved state. Starting items are linked to carried stacks at BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Equipment", meta = (TitleProperty = "Slot"))
	TArray<FFVEquippedItem> Equipped;

private:
	UFUNCTION()
	void HandleStackChanged(FGuid InstanceId);

	UFUNCTION()
	void HandleActorDataLoaded();

	/** Finds a carried, unworn stack of the item (or none, when items may be worn without carrying them). */
	bool ResolveInstance(const UFVItemDefinition* Item, FGuid& OutInstanceId) const;
	void LinkStartingInstances();

	/** Picks the slot and the slots that must be emptied; also checks conditions and the overflow rule. */
	bool PlanEquip(const UFVItemDefinition* Item, const FGuid& InstanceId, FGameplayTag RequestedSlot, FGameplayTag& OutSlot, TArray<FGameplayTag>& OutRemoved) const;
	bool FitsCapacity(const TArray<FGameplayTag>& Removed, const UFVItemDefinition* Added) const;
	void Apply(FGameplayTag Slot, UFVItemDefinition* Item, const FGuid& InstanceId, const TArray<FGameplayTag>& Removed);

	const FFVEquippedItem* FindEntry(const FGameplayTag& Slot) const;
	void RemoveAppliedTags();

	TWeakObjectPtr<UAbilitySystemComponent> AppliedTo;
	FGameplayTagContainer AppliedTags;
};
