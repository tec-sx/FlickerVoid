// A key hint: the label shown next to the key currently mapped to Action.
struct FFVContextHint
{
    UPROPERTY(EditAnywhere, Category = "Hints")
    UInputAction Action;

    UPROPERTY(EditAnywhere, Category = "Hints", meta = (ToolTip = "Empty uses the action's display name."))
    FText Label;
}

// One key hint: the glyph of the key mapped to an action for the current device (Common Input controller data), and a
// label. KeyText shows the key's name instead while the controller data has no glyph for that key.
UCLASS(Abstract)
class UFVInputHintWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UCommonActionWidget KeyIcon;

    UPROPERTY(BindWidget)
    UTextBlock LabelText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock KeyText;

    void SetHint(UInputAction Action, FText Label)
    {
        KeyIcon.SetEnhancedInputAction(Action);
        LabelText.SetText(Label.IsEmpty() ? KeyIcon.GetDisplayText() : Label);

        if (KeyText != nullptr)
        {
            bool bHasGlyph = KeyIcon.GetIcon().DrawAs != ESlateBrushDrawType::NoDrawType;
            KeyText.SetText(bHasGlyph ? FText() : FVUI::GetActionKeyText(GetOwningPlayer(), Action));
            KeyText.SetVisibility(bHasGlyph ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
        }
    }
}

// HUD key hints, at most three (e.g. Tab Hub, M Map, Esc Pause). Keys come from the active mapping contexts, so
// rebinding updates them; the glyph follows the device in use.
UCLASS(Abstract)
class UFVContextHintsWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Hints", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVContextHint> Hints;

    UPROPERTY(EditDefaultsOnly, Category = "Hints")
    TSubclassOf<UFVInputHintWidget> HintEntryClass;

    UPROPERTY(BindWidget)
    UPanelWidget HintList;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        Refresh();
        // Mapping contexts may be added after the HUD is built.
        System::SetTimer(this, n"Refresh", 0.5, false);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void Refresh()
    {
        HintList.ClearChildren();
        if (!HintEntryClass.IsValid())
        {
            return;
        }

        int Count = Math::Min(Hints.Num(), 3);
        for (int i = 0; i < Count; i++)
        {
            if (FVUI::GetKeysForAction(GetOwningPlayer(), Hints[i].Action).Num() == 0)
            {
                continue;
            }

            UFVInputHintWidget Widget = WidgetBlueprint::CreateWidget(HintEntryClass, GetOwningPlayer());
            Widget.SetHint(Hints[i].Action, Hints[i].Label);
            HintList.AddChild(Widget);
        }
    }
}
