#include "Layout/FVActivatableWidget.h"

#include "Engine/LocalPlayer.h"
#include "Input/CommonUIInputTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "InputAction.h"
#include "Layout/FVUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVActivatableWidget)

TOptional<FUIInputConfig> UFVActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputMode)
	{
	case EFVWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EFVWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EFVWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	default:
		return Super::GetDesiredInputConfig();
	}
}

void UFVActivatableWidget::BindInputAction(UInputAction* Action)
{
	if (!Action || BoundActions.Contains(Action))
	{
		return;
	}

	BoundActions.Add(Action);
	RegisterUIActionBinding(FBindUIActionArgs(Action, false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBoundAction, Action)));
}

void UFVActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetMenuInputActive(InputMode == EFVWidgetInputMode::Menu);
}

void UFVActivatableWidget::NativeOnDeactivated()
{
	SetMenuInputActive(false);
	Super::NativeOnDeactivated();
}

void UFVActivatableWidget::HandleBoundAction(UInputAction* Action)
{
	OnInputAction(Action);
}

void UFVActivatableWidget::SetMenuInputActive(const bool bActive)
{
	if (bMenuInputActive == bActive)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UFVUIManagerSubsystem* UIManager = LocalPlayer ? LocalPlayer->GetSubsystem<UFVUIManagerSubsystem>() : nullptr;
	if (!UIManager)
	{
		return;
	}

	bMenuInputActive = bActive;
	if (bActive)
	{
		UIManager->AddMenuInput();
	}
	else
	{
		UIManager->RemoveMenuInput();
	}
}
