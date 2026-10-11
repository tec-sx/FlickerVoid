// One page of the hub menu (Inventory, Character, Journal). Builds its sections from the Sections class list into
// SectionPanel: a Widget Switcher shows them as tabs (with SectionTabs), any other panel shows them side by side.
UCLASS(Abstract)
class UFVHubMenuPageWidget : UFVActivatableWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Page")
    FText PageTitle;

    UPROPERTY(EditDefaultsOnly, Category = "Page", meta = (MaxEntries = 3))
    TArray<TSubclassOf<UFVHubMenuSectionWidget>> Sections;

    UPROPERTY(BindWidget)
    UPanelWidget SectionPanel;

    UPROPERTY(BindWidgetOptional)
    UFVTabBarWidget SectionTabs;

    private UFVHubMenuWidget OwningHub;
    private TArray<UFVHubMenuSectionWidget> SectionWidgets;
    private UWidgetSwitcher SectionSwitcher;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        SectionSwitcher = Cast<UWidgetSwitcher>(SectionPanel);
        SectionPanel.ClearChildren();

        TArray<FText> Titles;
        for (TSubclassOf<UFVHubMenuSectionWidget> SectionClass : Sections)
        {
            if (!SectionClass.IsValid())
            {
                continue;
            }

            UFVHubMenuSectionWidget Section = WidgetBlueprint::CreateWidget(SectionClass, GetOwningPlayer());
            SectionPanel.AddChild(Section);
            SectionWidgets.Add(Section);
            Titles.Add(Section.SectionTitle);
        }

        if (SectionTabs != nullptr)
        {
            SectionTabs.OnTabSelected.AddUFunction(this, n"HandleSectionTabSelected");
            SectionTabs.SetTabs(SectionSwitcher != nullptr ? Titles : TArray<FText>());
        }

        for (UFVHubMenuSectionWidget Section : SectionWidgets)
        {
            Section.InitializeSection(this);
        }
    }

    void SetHub(UFVHubMenuWidget Hub)
    {
        OwningHub = Hub;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UFVHubMenuWidget GetHub() const
    {
        return OwningHub;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    TArray<UFVHubMenuSectionWidget> GetSections() const
    {
        return SectionWidgets;
    }

    // Called by the hub whenever this page is shown.
    void ShowPage()
    {
        if (SectionTabs != nullptr && SectionSwitcher != nullptr && SectionTabs.GetSelectedIndex() < 0)
        {
            SectionTabs.SelectTab(0);
        }
        OnPageShown();
        RefreshSections();
    }

    void HidePage()
    {
        for (UFVHubMenuSectionWidget Section : SectionWidgets)
        {
            Section.OnSectionHidden();
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void RefreshSections()
    {
        for (UFVHubMenuSectionWidget Section : SectionWidgets)
        {
            Section.Refresh();
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void ShowSection(int Index)
    {
        if (SectionTabs != nullptr && SectionSwitcher != nullptr)
        {
            SectionTabs.SelectTab(Index);
        }
    }

    // Shown section for tabbed pages, otherwise the first one.
    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UFVHubMenuSectionWidget GetActiveSection() const
    {
        if (SectionSwitcher != nullptr)
        {
            return Cast<UFVHubMenuSectionWidget>(SectionSwitcher.GetActiveWidget());
        }
        return SectionWidgets.Num() > 0 ? SectionWidgets[0] : nullptr;
    }

    // Page-wide setup before the sections refresh, e.g. a shared model.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnPageShown() {}

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        UFVHubMenuSectionWidget Section = GetActiveSection();
        return Section != nullptr ? Section.GetFocusTarget() : nullptr;
    }

    UFUNCTION()
    private void HandleSectionTabSelected(int Index)
    {
        if (SectionSwitcher != nullptr && SectionWidgets.IsValidIndex(Index))
        {
            SectionSwitcher.SetActiveWidgetIndex(Index);
            SectionWidgets[Index].Refresh();
            RequestRefreshFocus();
        }
    }
}
