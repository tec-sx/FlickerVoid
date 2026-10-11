// The hub menu (Tab): pages from the Pages class list, switched by PageTabs or the Previous/Next Page actions (Q/E).
// Closes on Back or on the key that opened it (Close Actions). Pushed on UI.Layer.GameMenu.
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
    UCommonActivatableWidgetSwitcher PageSwitcher;

    UPROPERTY(BindWidgetOptional)
    UTextBlock PageTitleText;

    private TArray<UFVHubMenuPageWidget> PageWidgets;
    private int ShownPage = -1;
    private int RequestedPage = 0;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
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

    bool HandleKey(FKey Key) override
    {
        if (IsKeyForAction(Key, NextPageAction))
        {
            PageTabs.SelectNext();
            return true;
        }
        if (IsKeyForAction(Key, PreviousPageAction))
        {
            PageTabs.SelectPrevious();
            return true;
        }
        return Super::HandleKey(Key);
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
        }

        ShownPage = Index;
        PageSwitcher.SetActiveWidgetIndex(Index);

        if (PageTitleText != nullptr)
        {
            PageTitleText.SetText(PageWidgets[Index].PageTitle);
        }

        PageWidgets[Index].ShowPage();
        RequestRefreshFocus();
    }
}
