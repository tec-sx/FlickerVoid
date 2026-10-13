#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/FVItemEntryWidget.h"
#include "FVEquipmentSlotWidget.generated.h"

class UFVEquipmentComponent;
class UFVItemDefinition;

/**
 * Item entry showing what is worn in one equipment slot. Place it anywhere in a layout and set Equipment Slot on the
 * placed widget; it shows the owning player's equipment, or the one given to Set Equipment, and refreshes itself when
 * it changes. An empty slot can be covered by another worn item (the blocking item), e.g. a dress over Lower Body.
 */
UCLASS(Abstract)
class FVINVENTORYSYSTEM_API UFVEquipmentSlotWidget : public UFVItemEntryWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Shows this equipment instead of the owning player's, e.g. an NPC's on a trade screen; null goes back to the player's."))
	void SetEquipment(UFVEquipmentComponent* InEquipment);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	UFVEquipmentComponent* GetEquipment() const { return Equipment.Get(); }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "The worn item covering this slot while it is empty, if any."))
	UFVItemDefinition* GetBlockingItem() const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	void Refresh();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Inventory", meta = (ToolTip = "Called after On Item Changed. Blocking Item is set when the slot is empty because another worn item covers it."))
	void OnSlotChanged(UFVItemDefinition* BlockingItem);

private:
	void Bind(UFVEquipmentComponent* InEquipment);
	void Unbind();

	UFUNCTION()
	void HandleEquipmentChanged(FGameplayTag ChangedSlot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem);

	UFUNCTION()
	void HandleEquipmentReset();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot", meta = (Categories = "Equipment.Slot"))
	FGameplayTag EquipmentSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot", meta = (ToolTip = "Name of the slot for the Blueprint to show, e.g. in Pre Construct so it shows in the designer."))
	FText SlotLabel;

private:
	TWeakObjectPtr<UFVEquipmentComponent> Equipment;
	TWeakObjectPtr<UFVEquipmentComponent> EquipmentOverride;
};
