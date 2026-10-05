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
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnEquipmentChanged, FGameplayTag, Slot, UFVItemDefinition*, OldItem, UFVItemDefinition*, NewItem);

/**
 * Wears equippable items (outfits, accessories), one per slot. Granted tags of worn items are added
 * to the owner's ability system as loose tags, so conditions and AI can read disguises and dress codes.
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

	/** Replaces whatever is in the item's slot. */
	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	bool Equip(UFVItemDefinition* Item);

	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	bool Unequip(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	UFVItemDefinition* GetEquipped(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	bool IsEquipped(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	const TArray<FFVEquippedItem>& GetAllEquipped() const { return Equipped; }

	/** Union of the granted tags of everything worn. */
	UFUNCTION(BlueprintPure, Category = "FV|Equipment")
	FGameplayTagContainer GetGrantedTags() const;

	/** Re-applies granted tags; call when the owner's ability system becomes available after BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "FV|Equipment")
	void RefreshGrantedTags();

	UPROPERTY(BlueprintAssignable, Category = "FV|Equipment")
	FFVOnEquipmentChanged OnEquipmentChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** If set, only items held in the owner's inventory can be equipped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	bool bRequireInventory = true;

	/** Starting equipment; also the saved state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Equipment", meta = (TitleProperty = "Slot"))
	TArray<FFVEquippedItem> Equipped;

private:
	UFUNCTION()
	void HandleInventoryChanged(UFVItemDefinition* Item, int32 OldQuantity, int32 NewQuantity);

	void SetSlot(FGameplayTag Slot, UFVItemDefinition* Item);
	void RemoveAppliedTags();

	TWeakObjectPtr<UAbilitySystemComponent> AppliedTo;
	FGameplayTagContainer AppliedTags;
};
