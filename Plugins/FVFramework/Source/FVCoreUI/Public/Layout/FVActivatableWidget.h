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

class UInputAction;

/**
 * Activatable widget that picks its input config from a property, so screens set Game or Menu without overriding a
 * function. Menu-mode widgets add the menu mapping context (FlickerVoid > UI) while active and receive bound input
 * actions through CommonUI, whatever has keyboard focus.
 */
UCLASS(Abstract, Blueprintable)
class FVCOREUI_API UFVActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UFUNCTION(BlueprintCallable, Category = "FV|UI", meta = (ToolTip = "Calls On Input Action whenever the action's key is pressed while this widget is in the active screen. Needs Enable Enhanced Input Support in Common Input settings and the action mapped in an active context, e.g. the menu context."))
	void BindInputAction(UInputAction* Action);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|UI", meta = (ToolTip = "A bound input action was pressed."))
	void OnInputAction(UInputAction* Action);

private:
	void HandleBoundAction(UInputAction* Action);
	void SetMenuInputActive(bool bActive);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Input config applied while this widget is the active one."))
	EFVWidgetInputMode InputMode = EFVWidgetInputMode::Default;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ToolTip = "Mouse capture while InputMode is Game or GameAndMenu."))
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInputAction>> BoundActions;

	bool bMenuInputActive = false;
};
