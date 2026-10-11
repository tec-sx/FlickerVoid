// A key hint: the label shown next to the key currently mapped to Action.
struct FFVContextHint
{
    UPROPERTY(EditAnywhere, Category = "Hints")
    UInputAction Action;

    UPROPERTY(EditAnywhere, Category = "Hints")
    FText Label;
}

// HUD key hints, at most three (e.g. Tab Hub, M Map, Esc Pause). Keys come from the active mapping contexts, so
// rebinding updates them.
UCLASS(Abstract)
class UFVContextHintsWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Hints", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVContextHint> Hints;

    UPROPERTY(EditDefaultsOnly, Category = "Hints")
    TSubclassOf<UFVListEntryWidget> HintEntryClass;

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
            FText Key = FVUI::GetActionKeyText(GetOwningPlayer(), Hints[i].Action);
            if (Key.IsEmpty())
            {
                continue;
            }

            FFVListEntry Entry;
            Entry.Title = Hints[i].Label;
            Entry.Value = Key;

            UFVListEntryWidget Widget = WidgetBlueprint::CreateWidget(HintEntryClass, GetOwningPlayer());
            Widget.SetEntry(Entry, i);
            HintList.AddChild(Widget);
        }
    }
}
