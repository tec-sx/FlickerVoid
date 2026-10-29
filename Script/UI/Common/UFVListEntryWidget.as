// What one row of a menu list shows. Sections fill these from their data source; Source and Id point back to it.
struct FFVListEntry
{
    UPROPERTY()
    FText Title;

    UPROPERTY()
    FText Subtitle;

    UPROPERTY()
    FText Value;

    UPROPERTY()
    FText Description;

    UPROPERTY()
    TSoftObjectPtr<UTexture2D> Icon;

    // 0 to 1 fills the row's progress bar; negative hides it.
    UPROPERTY()
    float Progress = -1.0;

    // Worn, active, completed: drawn emphasised by the row's Blueprint.
    UPROPERTY()
    bool bHighlighted = false;

    // Locked, failed or unavailable: drawn faded by the row's Blueprint.
    UPROPERTY()
    bool bDimmed = false;

    UPROPERTY()
    UObject Source;

    UPROPERTY()
    FGuid Id;
}

event void FFVOnListEntryClicked(UFVListEntryWidget Entry);

// One row of a menu list: title, optional subtitle, value, icon and progress bar. Style the states in OnEntryChanged and OnSelectedChanged.
UCLASS(Abstract)
class UFVListEntryWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UButton EntryButton;

    UPROPERTY(BindWidgetOptional)
    UTextBlock SubtitleText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ValueText;

    UPROPERTY(BindWidgetOptional)
    UImage IconImage;

    UPROPERTY(BindWidgetOptional)
    UProgressBar ProgressBar;

    UPROPERTY()
    FFVOnListEntryClicked OnClicked;

    private FFVListEntry Entry;
    private int EntryIndex = -1;
    private bool bSelected = false;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        EntryButton.OnClicked.AddUFunction(this, n"HandleClicked");
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SetEntry(FFVListEntry InEntry, int InIndex)
    {
        Entry = InEntry;
        EntryIndex = InIndex;

        SetOptionalText(SubtitleText, Entry.Subtitle);
        SetOptionalText(ValueText, Entry.Value);

        if (IconImage != nullptr)
        {
            if (Entry.Icon.IsNull())
            {
                IconImage.SetVisibility(ESlateVisibility::Collapsed);
            }
            else
            {
                IconImage.SetBrushFromSoftTexture(Entry.Icon, false);
                IconImage.SetVisibility(ESlateVisibility::HitTestInvisible);
            }
        }

        if (ProgressBar != nullptr)
        {
            ProgressBar.SetVisibility(Entry.Progress < 0.0 ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
            ProgressBar.SetPercent(float32(Math::Clamp(Entry.Progress, 0.0, 1.0)));
        }

        OnEntryChanged(Entry);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SetSelected(bool bInSelected)
    {
        if (bSelected == bInSelected)
        {
            return;
        }
        bSelected = bInSelected;
        OnSelectedChanged(bSelected);
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    FFVListEntry GetEntry() const
    {
        return Entry;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int GetEntryIndex() const
    {
        return EntryIndex;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    bool IsSelected() const
    {
        return bSelected;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    bool IsEmpty() const
    {
        return Entry.Source == nullptr && !Entry.Id.IsValid() && Entry.Title.IsEmpty();
    }

    // Style highlighted and dimmed rows here.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnEntryChanged(FFVListEntry NewEntry) {}

    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnSelectedChanged(bool bNowSelected) {}

    UFUNCTION()
    private void HandleClicked()
    {
        OnClicked.Broadcast(this);
    }

    private void SetOptionalText(UTextBlock Block, FText Text)
    {
        if (Block == nullptr)
        {
            return;
        }
        Block.SetText(Text);
        Block.SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    }
}
