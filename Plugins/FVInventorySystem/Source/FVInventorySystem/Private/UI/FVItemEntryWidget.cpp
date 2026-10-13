#include "UI/FVItemEntryWidget.h"

#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "UI/FVItemGridWidget.h"

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

void UFVItemEntryWidget::SetGrid(UFVItemGridWidget* InGrid, const int32 InIndex)
{
	Grid = InGrid;
	Index = InIndex;
}

void UFVItemEntryWidget::NativeDestruct()
{
	// The hub reuses this object after its Slate widget is gone, and no mouse leave arrives for a destroyed widget.
	bPressed = false;
	SetHoverState(false, false);
	Super::NativeDestruct();
}

void UFVItemEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// Slate sends still mouse events while a list scrolls under the cursor; only a real move takes focus.
	if (!InMouseEvent.GetCursorDelta().IsNearlyZero())
	{
		FocusFromMouse();
	}
	SetHoverState(true, bFocused);
}

FReply UFVItemEntryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.GetCursorDelta().IsNearlyZero())
	{
		FocusFromMouse();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
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
		// The mouse may rest on this cell while keyboard focus moved on; the click brings it back.
		FocusFromMouse();
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

FReply UFVItemEntryWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// The second press of a quick double click arrives here instead of Mouse Button Down.
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
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

FNavigationReply UFVItemEntryWidget::NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	if (const UFVItemGridWidget* OwningGrid = Grid.Get())
	{
		if (UWidget* Target = OwningGrid->FindNeighbour(Index, InNavigationEvent.GetNavigationType()))
		{
			return FNavigationReply::Explicit(Target->TakeWidget());
		}
	}
	return Super::NativeOnNavigation(MyGeometry, InNavigationEvent, InDefaultReply);
}

void UFVItemEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	SetHoverState(bMouseOver, true);

	// Keys, gamepad and the screen's initial focus scroll the cell into view; the mouse doesn't, or the list would jump.
	if (!bFocusingFromMouse && InFocusEvent.GetCause() != EFocusCause::Mouse)
	{
		if (const UFVItemGridWidget* OwningGrid = Grid.Get())
		{
			OwningGrid->ScrollIntoView(this);
		}
	}
}

void UFVItemEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	SetHoverState(bMouseOver, false);
}

void UFVItemEntryWidget::FocusFromMouse()
{
	if (FollowsMouseWithFocus() && !HasAnyUserFocus())
	{
		bFocusingFromMouse = true;
		SetFocus();
		bFocusingFromMouse = false;
	}
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
