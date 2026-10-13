// Base for full screens (hub menu, map, menus): takes UI input while active and adds the menu mapping context
// (FlickerVoid > UI > Menu Input Context). Its input actions arrive through CommonUI whatever has focus, so the
// keys work even when the mouse moved focus away. Close Actions (e.g. IA_Menu_Back on Escape) and CommonUI Back go to Back().
UCLASS(Abstract)
class UFVScreenWidget : UFVActivatableWidget
{
    default InputMode = EFVWidgetInputMode::Menu;
    default bIsBackHandler = true;
    default bIsFocusable = true;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (MaxEntries = 3))
    TArray<UInputAction> CloseActions;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        for (UInputAction Action : CloseActions)
        {
            BindInputAction(Action);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnInputAction(UInputAction Action)
    {
        HandleAction(Action);
    }

    UFUNCTION(BlueprintOverride)
    bool OnHandleBackAction()
    {
        Back();
        return true;
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void Close()
    {
        if (!IsActivated())
        {
            return;
        }
        OnClosing();
        DeactivateWidget();
    }

    // Escape, gamepad B or a Close Action. Closes the screen; screens with inner panels go back a level first.
    void Back()
    {
        Close();
    }

    // Runs before the screen deactivates through Close (Close Actions, Back, buttons).
    void OnClosing() {}

    // Subclasses bind their own actions with BindInputAction, handle them here first and fall back to this.
    bool HandleAction(UInputAction Action)
    {
        if (CloseActions.Contains(Action))
        {
            Back();
            return true;
        }
        return false;
    }
}
