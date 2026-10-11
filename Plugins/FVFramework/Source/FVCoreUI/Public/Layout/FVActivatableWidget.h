#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "FVActivatableWidget.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "WidgetInputMode"))
enum class EFVWidgetInputMode : uint8
{
	Default		UMETA(ToolTip = "Leaves the input config to whatever is active."),
	Game		UMETA(ToolTip = "Game input only; the mouse stays captured. For HUDs and overlays."),
	GameAndMenu	UMETA(ToolTip = "Game and UI input together."),
	Menu		UMETA(ToolTip = "UI input only; the cursor shows and the game stops receiving input. For screens and menus.")
};

/** Activatable widget that picks its input config from a property, so screens set Game or Menu without overriding a function. */
UCLASS(Abstract, Blueprintable)
class FVCOREUI_API UFVActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Input config applied while this widget is the active one."))
	EFVWidgetInputMode InputMode = EFVWidgetInputMode::Default;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Mouse capture while InputMode is Game or GameAndMenu."))
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
