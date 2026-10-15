// A hub section that lists entries (quests, notes, factions, skills...) with up to three filter tabs and a detail
// panel for the selected entry. Subclasses supply the filters and entries; selection, pooling and detail are handled here.
UCLASS(Abstract)
class UFVHubListSectionWidget : UFVHubSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "List")
    TSubclassOf<UFVListEntryWidget> EntryClass;

    UPROPERTY(EditDefaultsOnly, Category = "List")
    FText EmptyText;

    UPROPERTY(BindWidget)
    UPanelWidget EntryList;

    UPROPERTY(BindWidgetOptional)
    UFVTabBarWidget FilterTabs;

    UPROPERTY(BindWidgetOptional)
    UTextBlock EmptyListText;

    UPROPERTY(BindWidgetOptional)
    UWidget DetailPanel;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DetailTitleText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DetailSubtitleText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DetailValueText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DetailDescriptionText;

    UPROPERTY(BindWidgetOptional)
    UImage DetailIcon;

    UPROPERTY(BindWidgetOptional)
    UProgressBar DetailProgressBar;

    private TArray<UFVListEntryWidget> EntryWidgets;
    private TArray<FFVListEntry> Entries;
    private int SelectedIndex = -1;
    private int ActiveFilter = 0;

    UFUNCTION(BlueprintOverride)
    void OnSectionInitialized()
    {
        if (FilterTabs != nullptr)
        {
            FilterTabs.OnTabSelected.AddUFunction(this, n"HandleFilterSelected");
            FilterTabs.SetTabs(GetFilterLabels());
            FilterTabs.SelectTab(0);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnRefresh()
    {
        RebuildList();
    }

    UFUNCTION(BlueprintOverride)
    UWidget GetFocusTarget()
    {
        if (EntryWidgets.IsValidIndex(SelectedIndex))
        {
            return EntryWidgets[SelectedIndex];
        }
        return EntryWidgets.Num() > 0 ? EntryWidgets[0] : nullptr;
    }

    TArray<FText> GetFilterLabels()
    {
        return TArray<FText>();
    }

    UFUNCTION(BlueprintEvent)
    int GetSlotCount()
    {
        return 0;
    }

    TArray<FFVListEntry> BuildEntries(int Filter)
    {
        return TArray<FFVListEntry>();
    }

    // Extra detail for subclasses (objectives, actions...); the shared fields are already filled.
    void OnEntrySelected(FFVListEntry Entry) {}

    void OnSelectionCleared() {}

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int GetActiveFilter() const
    {
        return ActiveFilter;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    bool GetSelectedEntry(FFVListEntry& OutEntry) const
    {
        if (!Entries.IsValidIndex(SelectedIndex))
        {
            return false;
        }
        OutEntry = Entries[SelectedIndex];
        return true;
    }

    // Rebuilds the rows, keeping the selection on the same Source or Id when it is still listed.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void RebuildList()
    {
        FFVListEntry Previous;
        bool bHadSelection = GetSelectedEntry(Previous);

        Entries = BuildEntries(ActiveFilter);
        int Rows = Math::Max(Entries.Num(), GetSlotCount());
        EnsureEntryWidgets(Rows);

        for (int i = 0; i < EntryWidgets.Num(); i++)
        {
            EntryWidgets[i].SetVisibility(i < Rows ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
            EntryWidgets[i].SetSelected(false);
            if (i < Rows)
            {
                EntryWidgets[i].SetEntry(i < Entries.Num() ? Entries[i] : FFVListEntry(), i);
            }
        }

        if (EmptyListText != nullptr)
        {
            EmptyListText.SetText(EmptyText);
            EmptyListText.SetVisibility(Entries.Num() == 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }

        int NewSelection = Entries.Num() > 0 ? 0 : -1;
        if (bHadSelection)
        {
            for (int i = 0; i < Entries.Num(); i++)
            {
                if (IsSameEntry(Entries[i], Previous))
                {
                    NewSelection = i;
                    break;
                }
            }
        }

        SelectedIndex = -1;
        SelectEntry(NewSelection);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectEntry(int Index)
    {
        if (EntryWidgets.IsValidIndex(SelectedIndex))
        {
            EntryWidgets[SelectedIndex].SetSelected(false);
        }

        SelectedIndex = Entries.IsValidIndex(Index) ? Index : -1;
        if (SelectedIndex < 0 || !EntryWidgets.IsValidIndex(SelectedIndex))
        {
            SelectedIndex = -1;
            ShowDetail(FFVListEntry(), false);
            OnSelectionCleared();
            return;
        }

        EntryWidgets[SelectedIndex].SetSelected(true);
        ShowDetail(Entries[SelectedIndex], true);
        OnEntrySelected(Entries[SelectedIndex]);
    }

    UFUNCTION()
    private void HandleFilterSelected(int Index)
    {
        ActiveFilter = Index;
        SelectedIndex = -1;
        RebuildList();
    }

    UFUNCTION()
    private void HandleEntryClicked(UFVListEntryWidget Entry)
    {
        SelectEntry(Entry.GetEntryIndex());
    }

    private void EnsureEntryWidgets(int Count)
    {
        if (!EntryClass.IsValid())
        {
            if (Count > 0)
            {
                Print(f"{GetName()}: Entry Class is not set.");
            }
            return;
        }

        while (EntryWidgets.Num() < Count)
        {
            UFVListEntryWidget Widget = WidgetBlueprint::CreateWidget(EntryClass, GetOwningPlayer());
            Widget.OnClicked.AddUFunction(this, n"HandleEntryClicked");
            EntryList.AddChild(Widget);
            EntryWidgets.Add(Widget);
        }
    }

    private void ShowDetail(FFVListEntry Entry, bool bVisible)
    {
        if (DetailPanel != nullptr)
        {
            DetailPanel.SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
        }

        SetDetailText(DetailTitleText, Entry.Title);
        SetDetailText(DetailSubtitleText, Entry.Subtitle);
        SetDetailText(DetailValueText, Entry.Value);
        SetDetailText(DetailDescriptionText, Entry.Description);

        if (DetailIcon != nullptr)
        {
            DetailIcon.SetVisibility(Entry.Icon.IsNull() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
            if (!Entry.Icon.IsNull())
            {
                DetailIcon.SetBrushFromSoftTexture(Entry.Icon, false);
            }
        }

        if (DetailProgressBar != nullptr)
        {
            DetailProgressBar.SetVisibility(Entry.Progress < 0.0 ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
            DetailProgressBar.SetPercent(float32(Math::Clamp(Entry.Progress, 0.0, 1.0)));
        }
    }

    private void SetDetailText(UTextBlock Block, FText Text)
    {
        if (Block != nullptr)
        {
            Block.SetText(Text);
            Block.SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
        }
    }

    private bool IsSameEntry(FFVListEntry A, FFVListEntry B) const
    {
        if (A.Id.IsValid() || B.Id.IsValid())
        {
            return A.Id == B.Id;
        }
        return A.Source != nullptr && A.Source == B.Source;
    }
}
