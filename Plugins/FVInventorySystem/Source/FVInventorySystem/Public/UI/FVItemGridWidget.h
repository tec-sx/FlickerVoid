#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVItemGridWidget.generated.h"

class UFVItemEntryWidget;
class UPanelWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnItemGridEvent, UFVItemEntryWidget*, Entry, const FFVItemStack&, Item);

/**
 * Cells of item entries for any list of stacks: the inventory, a container's loot, a trader's stock. Creates Cell Count
 * entries in ItemPanel up front (also in the designer) and fills them in order from Set Items; more items than cells add
 * cells. Tracks the hovered entry (mouse or keyboard/gamepad focus) and the selected one, kept by instance id when the
 * items change. Keyboard and gamepad move between cells with the usual UI navigation.
 */
UCLASS(Abstract)
class FVINVENTORYSYSTEM_API UFVItemGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Fills the cells in order; the rest show empty. Keeps the selection if its stack is still there, and reports the hovered cell again with its new item."))
	void SetItems(const TArray<FFVItemStack>& Items);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	void ClearItems();

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	TArray<UFVItemEntryWidget*> GetEntries() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "The entry under the mouse or holding keyboard/gamepad focus; null when none."))
	UFVItemEntryWidget* GetHoveredEntry() const { return HoveredEntry.Get(); }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "False when nothing is hovered or the hovered cell is empty."))
	bool GetHoveredItem(FFVItemStack& OutItem) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	UFVItemEntryWidget* GetSelectedEntry() const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool GetSelectedItem(FFVItemStack& OutItem) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Selects the cell holding this stack; an id not in the grid clears the selection."))
	void SelectItem(FGuid InstanceId);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	void ClearSelection();

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Where keyboard/gamepad focus should land: the selected cell, else the first one. Return it from the page's focus target."))
	UWidget* GetFocusTarget() const;

protected:
	virtual void NativePreConstruct() override;

private:
	void EnsureCells(int32 Count);
	void RemoveLastCell();
	void SetSelectedEntry(UFVItemEntryWidget* Entry);
	UFVItemEntryWidget* FindEntry(const FGuid& InstanceId) const;

	UFUNCTION()
	void HandleEntryHovered(UFVItemEntryWidget* Entry);

	UFUNCTION()
	void HandleEntryUnhovered(UFVItemEntryWidget* Entry);

	UFUNCTION()
	void HandleEntryClicked(UFVItemEntryWidget* Entry);

public:
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "The hovered cell changed, or its item did. Entry is null when nothing is hovered; Item has no Item for an empty cell."))
	FFVOnItemGridEvent OnHoveredItemChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "Click or accept on a cell, empty ones included. A filled cell is selected first."))
	FFVOnItemGridEvent OnItemClicked;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "Entry is null when the selection was cleared."))
	FFVOnItemGridEvent OnSelectionChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Grid", meta = (ToolTip = "Widget created for each cell."))
	TSubclassOf<UFVItemEntryWidget> EntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Grid", meta = (ClampMin = 0, ToolTip = "Cells created up front, shown empty until items fill them. Set it per placed grid: many for the inventory, a few for loot."))
	int32 CellCount = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Grid", meta = (ClampMin = 1, ToolTip = "Cells per row when ItemPanel is a Uniform Grid Panel; other panels (Wrap Box, Vertical Box) lay the cells out themselves."))
	int32 Columns = 5;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "FV|Inventory", meta = (BindWidget, ToolTip = "Holds the cells; put it in a Scroll Box for long lists. Its children are replaced at runtime and in the designer."))
	TObjectPtr<UPanelWidget> ItemPanel;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVItemEntryWidget>> Entries;

	TWeakObjectPtr<UFVItemEntryWidget> HoveredEntry;
	FGuid SelectedId;
};
