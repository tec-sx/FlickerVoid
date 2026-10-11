#include "Layout/FVActivatableWidget.h"

#include "Input/UIActionBindingHandle.h"

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
