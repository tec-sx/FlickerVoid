// The hub menu (Tab): pages from the Pages class list, switched by PageTabs or the Previous/Next Page actions (Tab/E, Q)
// through an animated switcher (transition type, curve and duration are set on PageSwitcher in the Widget Blueprint).
// Closes on Back (Escape) or its Close Actions. Pushed on UI.Layer.GameMenu.
UCLASS(Abstract)
class UFVHubMenuWidget : UFVScreenWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Hub", meta = (MaxEntries = 3))
    TArray<TSubclassOf<UFVHubMenuPageWidget>> Pages;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction NextPageAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction PreviousPageAction;

    UPROPERTY(BindWidget)
    UFVTabBarWidget PageTabs;

    UPROPERTY(BindWidget)
    UCommonAnimatedSwitcher PageSwitcher;

    UPROPERTY(BindWidgetOptional)
    UTextBlock PageTitleText;

    private TArray<UFVHubMenuPageWidget> PageWidgets;
    private int ShownPage = -1;
    private int RequestedPage = 0;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        Super::OnInitialized();
        BindInputAction(NextPageAction);
        BindInputAction(PreviousPageAction);
        PageSwitcher.ClearChildren();

        TArray<FText> Titles;
        for (TSubclassOf<UFVHubMenuPageWidget> PageClass : Pages)
        {
            if (!PageClass.IsValid())
            {
                continue;
            }

            UFVHubMenuPageWidget Page = WidgetBlueprint::CreateWidget(PageClass, GetOwningPlayer());
            Page.SetHub(this);
            PageSwitcher.AddChild(Page);
            PageWidgets.Add(Page);
            Titles.Add(Page.PageTitle);
        }

        PageTabs.OnTabSelected.AddUFunction(this, n"HandlePageSelected");
        PageTabs.SetTabs(Titles);
    }

    UFUNCTION(BlueprintOverride)
    void OnActivated()
    {
        if (PageWidgets.Num() == 0)
        {
            Print(f"{GetName()}: no hub pages; fill Pages in the class defaults.");
            return;
        }

        int Page = Math::Clamp(RequestedPage, 0, PageWidgets.Num() - 1);
        if (Page == PageTabs.GetSelectedIndex())
        {
            ShowPage(Page);
        }
        else
        {
            PageTabs.SelectTab(Page);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnDeactivated()
    {
        if (PageWidgets.IsValidIndex(ShownPage))
        {
            PageWidgets[ShownPage].HidePage();
            PageWidgets[ShownPage].DeactivateWidget();
        }
        RequestedPage = Math::Max(ShownPage, 0);
    }

    // Opens on this page the next time the hub is shown, or switches now if it is open.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void OpenPage(TSubclassOf<UFVHubMenuPageWidget> PageClass)
    {
        for (int i = 0; i < PageWidgets.Num(); i++)
        {
            if (PageWidgets[i].Class == PageClass.Get())
            {
                RequestedPage = i;
                if (IsActivated())
                {
                    PageTabs.SelectTab(i);
                }
                return;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UFVHubMenuPageWidget GetShownPage() const
    {
        return PageWidgets.IsValidIndex(ShownPage) ? PageWidgets[ShownPage] : nullptr;
    }

    bool HandleAction(UInputAction Action) override
    {
        if (Action != nullptr && Action == NextPageAction)
        {
            PageTabs.SelectNext();
            return true;
        }
        if (Action != nullptr && Action == PreviousPageAction)
        {
            PageTabs.SelectPrevious();
            return true;
        }
        return Super::HandleAction(Action);
    }

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        UFVHubMenuPageWidget Page = GetShownPage();
        return Page != nullptr ? Page.GetDesiredFocusTarget() : nullptr;
    }

    UFUNCTION()
    private void HandlePageSelected(int Index)
    {
        ShowPage(Index);
    }

    private void ShowPage(int Index)
    {
        if (!PageWidgets.IsValidIndex(Index))
        {
            return;
        }

        if (PageWidgets.IsValidIndex(ShownPage) && ShownPage != Index)
        {
            PageWidgets[ShownPage].HidePage();
            PageWidgets[ShownPage].DeactivateWidget();
        }

        ShownPage = Index;
        PageSwitcher.SetActiveWidgetIndex(Index);
        if (!PageWidgets[Index].IsActivated())
        {
            PageWidgets[Index].ActivateWidget();
        }

        if (PageTitleText != nullptr)
        {
            PageTitleText.SetText(PageWidgets[Index].PageTitle);
        }

        PageWidgets[Index].ShowPage();
        RequestRefreshFocus();
    }
}
