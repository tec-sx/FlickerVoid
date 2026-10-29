// One page of the hub menu (Inventory, Character, Journal), laid out by hand in its Widget Blueprint; nothing in the
// layout is required. The sections placed in it (side by side, or in an FV Tab View) are found when the page is created,
// refreshed when it is shown and told when it is hidden. Gamepad and keyboard focus goes to the first section on screen
// that has a focus target, unless Get Page Focus Target is overridden.
UCLASS(Abstract)
class UFVHubPageWidget : UFVActivatableWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Page")
    FText PageTitle;

    private UFVHubMenuWidget OwningHub;
    private TArray<UFVHubSectionWidget> SectionWidgets;
    private TArray<UFVTabViewWidget> TabViews;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        for (UWidget Widget : FVUI::FindWidgetsOfClass(this, UFVTabViewWidget))
        {
            UFVTabViewWidget TabView = Cast<UFVTabViewWidget>(Widget);
            TabView.OnTabChanged.AddUFunction(this, n"HandleTabChanged");
            TabViews.Add(TabView);
        }

        for (UWidget Widget : FVUI::FindWidgetsOfClass(this, UFVHubSectionWidget))
        {
            SectionWidgets.Add(Cast<UFVHubSectionWidget>(Widget));
        }

        if (SectionWidgets.Num() > 3)
        {
            Print(f"{GetName()}: {SectionWidgets.Num()} sections; keep it to 3 or fewer.");
        }

        for (UFVHubSectionWidget Section : SectionWidgets)
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
    TArray<UFVHubSectionWidget> GetSections() const
    {
        return SectionWidgets;
    }

    // Called by the hub whenever this page is shown.
    void ShowPage()
    {
        OnPageShown();
        RefreshSections();
    }

    void HidePage()
    {
        for (UFVHubSectionWidget Section : SectionWidgets)
        {
            Section.OnSectionHidden();
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void RefreshSections()
    {
        for (UFVHubSectionWidget Section : SectionWidgets)
        {
            Section.Refresh();
        }
    }

    // False when the widget sits in a tab of this page that isn't selected.
    UFUNCTION(BlueprintPure, Category = "FV|UI")
    bool IsOnShownTab(UWidget Widget) const
    {
        for (UFVTabViewWidget TabView : TabViews)
        {
            int Tab = TabView.FindTab(Widget);
            if (Tab >= 0 && Tab != TabView.GetSelectedIndex())
            {
                return false;
            }
        }
        return true;
    }

    // Page-wide setup before the sections refresh, e.g. a shared model.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnPageShown() {}

    // Where gamepad and keyboard focus lands when the page or one of its tabs is shown.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    UWidget GetPageFocusTarget() const
    {
        for (UFVHubSectionWidget Section : SectionWidgets)
        {
            if (IsOnShownTab(Section))
            {
                UWidget Target = Section.GetFocusTarget();
                if (Target != nullptr)
                {
                    return Target;
                }
            }
        }
        return nullptr;
    }

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        return GetPageFocusTarget();
    }

    UFUNCTION()
    private void HandleTabChanged(int Index, UWidget Content)
    {
        if (!IsActivated())
        {
            return;
        }
        for (UFVHubSectionWidget Section : SectionWidgets)
        {
            if (FVUI::IsWidgetInside(Section, Content))
            {
                Section.Refresh();
            }
        }
        RequestRefreshFocus();
    }
}