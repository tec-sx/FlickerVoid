// Base for full screens (hub menu, map, menus): takes UI input while active, closes on Back and on any key mapped
// to one of its Close Actions, so the key that opened a screen also closes it.
UCLASS(Abstract)
class UFVScreenWidget : UFVActivatableWidget
{
    default InputMode = EFVWidgetInputMode::Menu;
    default bIsBackHandler = true;
    default bIsFocusable = true;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TArray<UInputAction> CloseActions;

    UFUNCTION(BlueprintOverride)
    FEventReply OnKeyDown(FGeometry MyGeometry, FKeyEvent InKeyEvent)
    {
        if (!InKeyEvent.IsRepeat() && HandleKey(InKeyEvent.GetKey()))
        {
            return FEventReply::Handled();
        }
        return FEventReply::Unhandled();
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void Close()
    {
        OnClosing();
        DeactivateWidget();
    }

    // Runs before the screen deactivates through Close (Close Actions, buttons).
    void OnClosing() {}

    // Subclasses handle their own keys first and fall back to this.
    bool HandleKey(FKey Key)
    {
        if (IsKeyForAnyAction(Key, CloseActions))
        {
            Close();
            return true;
        }
        return false;
    }

    bool IsKeyForAction(FKey Key, UInputAction Action) const
    {
        return Action != nullptr && FVUI::IsKeyForAction(GetOwningPlayer(), Action, Key);
    }

    bool IsKeyForAnyAction(FKey Key, TArray<UInputAction> Actions) const
    {
        for (UInputAction Action : Actions)
        {
            if (IsKeyForAction(Key, Action))
            {
                return true;
            }
        }
        return false;
    }
}
