#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVItemEntryWidget.generated.h"

class UFVItemEntryWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnItemEntryEvent, UFVItemEntryWidget*, Entry);

/**
 * One item stack, or an empty cell. Has no required child widgets: the Blueprint shows the item in On Item Changed and
 * styles hover and selection in their events. Hovered means under the mouse or holding keyboard/gamepad focus; a click
 * or the accept key/button fires On Clicked. Used by FV Item Grid, or placed on its own.
 */
UCLASS(Abstract)
class FVINVENTORYSYSTEM_API UFVItemEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFVItemEntryWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	void SetItem(const FFVItemStack& InItem);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory", meta = (ToolTip = "Shows the entry as an empty cell."))
	void ClearItem();

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	const FFVItemStack& GetItem() const { return ItemStack; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool IsEmpty() const { return ItemStack.Item == nullptr; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Under the mouse or holding keyboard/gamepad focus."))
	bool IsItemHovered() const { return bMouseOver || bFocused; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool IsSelected() const { return bSelected; }

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	void SetSelected(bool bInSelected);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Cell index in its item grid; -1 for an entry placed on its own."))
	int32 GetIndex() const { return Index; }

	void SetIndex(int32 InIndex) { Index = InIndex; }

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Inventory", meta = (ToolTip = "Show the item here (icon, quantity, anything the design needs). bEmpty: an empty cell."))
	void OnItemChanged(const FFVItemStack& Item, bool bEmpty);

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Inventory")
	void OnHoverChanged(bool bHovered);

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Inventory")
	void OnSelectedChanged(bool bNowSelected);

private:
	void SetHoverState(bool bInMouseOver, bool bInFocused);
	void Click();

public:
	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory", meta = (ToolTip = "Left click, or the accept key/button while focused."))
	FFVOnItemEntryEvent OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnItemEntryEvent OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnItemEntryEvent OnUnhovered;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Entry", meta = (ToolTip = "The mouse moves keyboard/gamepad focus to the entry it enters, so one entry shows as hovered at a time and stays hovered when the mouse leaves for a button."))
	bool bFocusOnHover = true;

private:
	UPROPERTY(Transient)
	FFVItemStack ItemStack;

	int32 Index = INDEX_NONE;
	bool bMouseOver = false;
	bool bFocused = false;
	bool bSelected = false;
	bool bPressed = false;
};
