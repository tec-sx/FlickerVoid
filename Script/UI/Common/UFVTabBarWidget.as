event void FFVOnTabIndexSelected(int Index);

// One tab of a UFVTabBarWidget. A Common Button: its Common Button Style gives the selected, hovered and disabled looks,
// and gamepad focus moves between tabs like any Common Button.
UCLASS(Abstract)
class UFVTabButtonWidget : UCommonButtonBase
{
    UPROPERTY(BindWidget)
    UTextBlock LabelText;

    void SetLabel(FText Label)
    {
        LabelText.SetText(Label);
    }
}

// A row of tabs on CommonUI's tab list, built from labels. Owners call SetTabs (or SetContentTabs to drive a Common
// Animated Switcher), then listen to OnTabIndexSelected; Next and Previous wrap around.
UCLASS(Abstract)
class UFVTabBarWidget : UCommonTabListWidgetBase
{
    UPROPERTY(BindWidget)
    UPanelWidget TabPanel;

    UPROPERTY(EditAnywhere, Category = "Tabs")
    TSubclassOf<UFVTabButtonWidget> TabButtonClass;

    UPROPERTY()
    FFVOnTabIndexSelected OnTabIndexSelected;

    private TArray<FName> TabIds;
    private int SelectedIndex = -1;
    private bool bBuildingTabs = false;
    private bool bListening = false;

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SetTabs(TArray<FText> Labels)
    {
        TArray<UWidget> NoContents;
        BuildTabs(Labels, nullptr, NoContents);
    }

    // Selecting tab i shows Contents[i] in Switcher; the contents must be the switcher's children, one per label.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SetContentTabs(TArray<FText> Labels, UCommonAnimatedSwitcher Switcher, TArray<UWidget> Contents)
    {
        if (Switcher == nullptr || Contents.Num() != Labels.Num())
        {
            Print(f"{GetName()}: content tabs need a switcher and one content widget per label.");
            return;
        }
        BuildTabs(Labels, Switcher, Contents);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectTab(int Index)
    {
        if (!TabIds.IsValidIndex(Index) || Index == SelectedIndex)
        {
            return;
        }

        if (GetSelectedTabId() != TabIds[Index] && !SelectTabByID(TabIds[Index], false))
        {
            return;
        }
        ApplySelection(Index);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectNext()
    {
        if (TabIds.Num() > 0)
        {
            SelectTab((SelectedIndex + 1) % TabIds.Num());
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectPrevious()
    {
        if (TabIds.Num() > 0)
        {
            SelectTab((SelectedIndex - 1 + TabIds.Num()) % TabIds.Num());
        }
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int GetSelectedIndex() const
    {
        return SelectedIndex;
    }

    // The tab list selects its first tab while tabs are registered; that selection is reported by the owner's own
    // SelectTab call, so registering stays silent.
    private void BuildTabs(TArray<FText> Labels, UCommonAnimatedSwitcher Switcher, TArray<UWidget> Contents)
    {
        if (!bListening)
        {
            OnTabSelected.AddUFunction(this, n"HandleTabSelected");
            bListening = true;
        }

        bBuildingTabs = true;
        RemoveAllTabs();
        TabPanel.ClearChildren();
        TabIds.Empty();
        SelectedIndex = -1;
        SetLinkedSwitcher(Switcher);

        if (!TabButtonClass.IsValid())
        {
            Print(f"{GetName()}: Tab Button Class is not set.");
        }
        else
        {
            for (int i = 0; i < Labels.Num(); i++)
            {
                FName TabId = FName(f"Tab{i}");
                UWidget Content = Switcher != nullptr ? Contents[i] : nullptr;
                if (!RegisterTab(TabId, TabButtonClass, Content, -1))
                {
                    Print(f"{GetName()}: could not add tab {i}.");
                    break;
                }

                UFVTabButtonWidget Button = Cast<UFVTabButtonWidget>(GetTabButtonBaseByID(TabId));
                Button.SetLabel(Labels[i]);
                TabPanel.AddChild(Button);
                TabIds.Add(TabId);
            }
        }

        bBuildingTabs = false;
        SetVisibility(TabIds.Num() > 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }

    private void ApplySelection(int Index)
    {
        if (Index < 0 || Index == SelectedIndex)
        {
            return;
        }
        SelectedIndex = Index;
        OnTabIndexSelected.Broadcast(SelectedIndex);
    }

    UFUNCTION()
    private void HandleTabSelected(FName TabId)
    {
        if (!bBuildingTabs)
        {
            ApplySelection(TabIds.FindIndex(TabId));
        }
    }
}
