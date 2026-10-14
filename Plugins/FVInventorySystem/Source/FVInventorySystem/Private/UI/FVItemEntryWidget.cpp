#include "UI/FVItemEntryWidget.h"

#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemEntryWidget)

UFVItemEntryWidget::UFVItemEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Visible so the whole cell takes the mouse, not just its hit-testable children.
	SetVisibilityInternal(ESlateVisibility::Visible);
	SetIsFocusable(true);
}

void UFVItemEntryWidget::SetItem(const FFVItemStack& InItem)
{
	ItemStack = InItem;
	OnItemChanged(ItemStack, IsEmpty());
}

void UFVItemEntryWidget::ClearItem()
{
	SetItem(FFVItemStack());
}

void UFVItemEntryWidget::SetSelected(const bool bInSelected)
{
	if (bSelected != bInSelected)
	{
		bSelected = bInSelected;
		OnSelectedChanged(bSelected);
	}
}

void UFVItemEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// Focus first, so the entry losing focus reports its unhover before this one reports its hover.
	if (bFocusOnHover && IsFocusable() && !HasAnyUserFocus())
	{
		SetFocus();
	}
	SetHoverState(true, bFocused);
}

void UFVItemEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bPressed = false;
	SetHoverState(false, bFocused);
}

FReply UFVItemEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && GetIsEnabled())
	{
		bPressed = true;
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UFVItemEntryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bPressed)
	{
		bPressed = false;
		Click();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UFVItemEntryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!InKeyEvent.IsRepeat() && GetIsEnabled() && FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
	{
		Click();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UFVItemEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	SetHoverState(bMouseOver, true);
}

void UFVItemEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	SetHoverState(bMouseOver, false);
}

void UFVItemEntryWidget::SetHoverState(const bool bInMouseOver, const bool bInFocused)
{
	const bool bWasHovered = IsItemHovered();
	bMouseOver = bInMouseOver;
	bFocused = bInFocused;

	if (IsItemHovered() == bWasHovered)
	{
		return;
	}

	OnHoverChanged(!bWasHovered);
	if (bWasHovered)
	{
		OnUnhovered.Broadcast(this);
	}
	else
	{
		OnHovered.Broadcast(this);
	}
}

void UFVItemEntryWidget::Click()
{
	OnClicked.Broadcast(this);
}
