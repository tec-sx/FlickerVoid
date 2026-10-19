// The hub menu (Tab), laid out by hand: its pages are the FV Hub Menu Page widgets placed in the layout. With an FV Tab
// View bound as PageTabView (the pages as the children of its switcher) one page shows at a time and the Previous/Next Page
// actions (Tab/E, Q) switch them; without one every placed page shows. Closes on Back (Escape) or its Close Actions.
// Pushed on UI.Layer.GameMenu.
UCLASS(Abstract)
class UFVHubMenuWidget : UFVScreenWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction NextPageAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction PreviousPageAction;

    UPROPERTY(BindWidgetOptional)
    UFVTabViewWidget PageTabView;

    UPROPERTY(BindWidgetOptional)
    UTextBlock PageTitleText;

    private TArray<UFVHubPageWidget> PageWidgets;
    private TArray<UFVHubPageWidget> ShownPages;
    private UFVHubPageWidget RequestedPage;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        Super::OnInitialized();
        BindInputAction(NextPageAction, PageTabView != nullptr);
        BindInputAction(PreviousPageAction, PageTabView != nullptr);

        for (UWidget Widget : FVUI::FindWidgetsOfClass(this, UFVHubPageWidget))
        {
            UFVHubPageWidget Page = Cast<UFVHubPageWidget>(Widget);
            Page.SetHub(this);
            PageWidgets.Add(Page);
        }

        if (PageWidgets.Num() > 3)
        {
            Print(f"{GetName()}: {PageWidgets.Num()} hub pages; keep it to 3 or fewer.");
        }

        if (PageTabView != nullptr)
        {
            PageTabView.OnTabChanged.AddUFunction(this, n"HandlePageTabChanged");
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnActivated()
    {
        if (PageWidgets.Num() == 0)
        {
            Print(f"{GetName()}: no hub pages; place FV Hub Menu Page widgets in the layout.");
            return;
        }

        if (PageTabView == nullptr)
        {
            for (UFVHubPageWidget Page : PageWidgets)
            {
                ActivatePage(Page);
            }
            RequestRefreshFocus();
            return;
        }

        UFVHubPageWidget Page = RequestedPage != nullptr ? RequestedPage : FindPageOnTab(PageTabView.GetSelectedIndex());
        RequestedPage = nullptr;
        SelectPage(Page != nullptr ? Page : PageWidgets[0]);
    }

    UFUNCTION(BlueprintOverride)
    void OnDeactivated()
    {
        for (int i = ShownPages.Num() - 1; i >= 0; i--)
        {
            DeactivatePage(ShownPages[i]);
        }
    }

    // Opens on this page the next time the hub is shown, or switches now if it is open.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void OpenPage(TSubclassOf<UFVHubPageWidget> PageClass)
    {
        for (UFVHubPageWidget Page : PageWidgets)
        {
            if (Page.Class == PageClass.Get())
            {
                if (IsActivated() && PageTabView != nullptr)
                {
                    SelectPage(Page);
                }
                else
                {
                    RequestedPage = Page;
                }
                return;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UFVHubPageWidget GetShownPage() const
    {
        return ShownPages.Num() > 0 ? ShownPages[0] : nullptr;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    TArray<UFVHubPageWidget> GetPages() const
    {
        return PageWidgets;
    }

    bool HandleAction(UInputAction Action) override
    {
        if (Action != nullptr && Action == NextPageAction && PageTabView != nullptr)
        {
            PageTabView.SelectNext();
            return true;
        }
        if (Action != nullptr && Action == PreviousPageAction && PageTabView != nullptr)
        {
            PageTabView.SelectPrevious();
            return true;
        }
        return Super::HandleAction(Action);
    }

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        UFVHubPageWidget Page = GetShownPage();
        return Page != nullptr ? Page.GetDesiredFocusTarget() : nullptr;
    }

    // The tab view also selects its first tab when it is built, before the hub is active; OnActivated shows that page.
    UFUNCTION()
    private void HandlePageTabChanged(int Index, UWidget Content)
    {
        if (IsActivated())
        {
            ShowOnly(FindPageOnTab(Index));
        }
    }

    private void SelectPage(UFVHubPageWidget Page)
    {
        int Tab = PageTabView.FindTab(Page);
        if (Tab >= 0 && Tab != PageTabView.GetSelectedIndex())
        {
            PageTabView.SelectTab(Tab);
        }
        else
        {
            ShowOnly(Page);
        }
    }

    private void ShowOnly(UFVHubPageWidget Page)
    {
        for (int i = ShownPages.Num() - 1; i >= 0; i--)
        {
            if (ShownPages[i] != Page)
            {
                DeactivatePage(ShownPages[i]);
            }
        }

        if (Page != nullptr)
        {
            ActivatePage(Page);
        }
    }

    private void ActivatePage(UFVHubPageWidget Page)
    {
        ShownPages.AddUnique(Page);
        if (PageTitleText != nullptr)
        {
            PageTitleText.SetText(Page.PageTitle);
        }

        // Sections fill before activation, so the focus CommonUI sets on activation finds their rows.
        Page.ShowPage();
        if (!Page.IsActivated())
        {
            Page.ActivateWidget();
        }
        else
        {
            Page.RequestRefreshFocus();
        }
    }

    private void DeactivatePage(UFVHubPageWidget Page)
    {
        ShownPages.Remove(Page);
        Page.HidePage();
        Page.DeactivateWidget();
    }

    private UFVHubPageWidget FindPageOnTab(int Index) const
    {
        for (UFVHubPageWidget Page : PageWidgets)
        {
            if (Index >= 0 && PageTabView.FindTab(Page) == Index)
            {
                return Page;
            }
        }
        return nullptr;
    }
}