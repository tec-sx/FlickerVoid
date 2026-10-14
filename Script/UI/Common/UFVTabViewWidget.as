event void FFVOnTabChanged(int Index, UWidget Content);

// Tabs over content laid out by hand where the tab view is placed: put a Widget Switcher in its TabContent slot (a Common
// Animated Switcher animates the change) and lay the tabs out as its children, in order. Builds TabBar from Tab Labels,
// or from the children's Page Title or Section Title, keeps the bar and the switcher in step and reports On Tab Changed.
// Works in any layout (the hub, a page, the options screen); owners call Select Next / Select Previous from their input.
UCLASS(Abstract)
class UFVTabViewWidget : UUserWidget
{
    UPROPERTY(EditAnywhere, Category = "Tabs", meta = (MaxEntries = 3, ToolTip = "Tab labels in child order; an empty one uses the child's Page Title or Section Title."))
    TArray<FText> TabLabels;

    UPROPERTY(BindWidget)
    UFVTabBarWidget TabBar;

    UPROPERTY(BindWidget)
    UNamedSlot TabContent;

    UPROPERTY()
    FFVOnTabChanged OnTabChanged;

    private UWidgetSwitcher Switcher;
    private TArray<UWidget> Contents;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        Switcher = Cast<UWidgetSwitcher>(TabContent.GetContent());
        if (Switcher == nullptr)
        {
            Print(f"{GetName()}: put a Widget Switcher in the Tab Content slot.");
            return;
        }

        TArray<FText> Labels;
        for (int i = 0; i < Switcher.GetChildrenCount(); i++)
        {
            UWidget Content = Switcher.GetChildAt(i);
            Contents.Add(Content);
            Labels.Add(GetLabel(i, Content));
        }

        if (Contents.Num() > 3)
        {
            Print(f"{GetName()}: {Contents.Num()} tabs; keep it to 3 or fewer.");
        }

        TabBar.OnTabSelected.AddUFunction(this, n"HandleTabSelected");
        TabBar.SetTabs(Labels);
    }

    // Starts on the child the switcher shows in the designer.
    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        if (Switcher != nullptr && GetSelectedIndex() < 0)
        {
            SelectTab(Switcher.GetActiveWidgetIndex());
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectTab(int Index)
    {
        TabBar.SelectTab(Index);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectNext()
    {
        TabBar.SelectNext();
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectPrevious()
    {
        TabBar.SelectPrevious();
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int GetSelectedIndex() const
    {
        return TabBar.GetSelectedIndex();
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UWidget GetSelectedContent() const
    {
        int Index = GetSelectedIndex();
        return Contents.IsValidIndex(Index) ? Contents[Index] : nullptr;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    TArray<UWidget> GetContents() const
    {
        return Contents;
    }

    // Index of the tab that is the widget or holds it; -1 when the widget isn't in this tab view.
    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int FindTab(UWidget Widget) const
    {
        for (int i = 0; i < Contents.Num(); i++)
        {
            if (FVUI::IsWidgetInside(Widget, Contents[i]))
            {
                return i;
            }
        }
        return -1;
    }

    UFUNCTION()
    private void HandleTabSelected(int Index)
    {
        if (Switcher != nullptr && Contents.IsValidIndex(Index))
        {
            Switcher.SetActiveWidgetIndex(Index);
            OnTabChanged.Broadcast(Index, Contents[Index]);
        }
    }

    private FText GetLabel(int Index, UWidget Content) const
    {
        if (TabLabels.IsValidIndex(Index) && !TabLabels[Index].IsEmpty())
        {
            return TabLabels[Index];
        }

        UFVHubPageWidget Page = Cast<UFVHubPageWidget>(Content);
        if (Page != nullptr)
        {
            return Page.PageTitle;
        }

        UFVHubSectionWidget Section = Cast<UFVHubSectionWidget>(Content);
        if (Section != nullptr)
        {
            return Section.SectionTitle;
        }

        return FText::FromString(Content.GetName().ToString());
    }
}