event void FFVOnTabClicked(int Index);
event void FFVOnTabSelected(int Index);

// One tab of a UFVTabBarWidget. Style the selected state in OnSelectedChanged.
UCLASS(Abstract)
class UFVTabButtonWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UButton TabButton;

    UPROPERTY(BindWidget)
    UTextBlock LabelText;

    UPROPERTY()
    FFVOnTabClicked OnTabClicked;

    private int TabIndex = -1;
    private bool bSelected = false;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        TabButton.OnClicked.AddUFunction(this, n"HandleClicked");
    }

    void Setup(FText Label, int Index)
    {
        LabelText.SetText(Label);
        TabIndex = Index;
    }

    void SetSelected(bool bInSelected)
    {
        bSelected = bInSelected;
        OnSelectedChanged(bSelected);
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    bool IsSelected() const
    {
        return bSelected;
    }

    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnSelectedChanged(bool bNowSelected) {}

    UFUNCTION()
    private void HandleClicked()
    {
        OnTabClicked.Broadcast(TabIndex);
    }
}

// A row of tabs built from labels. Owners call SetTabs, then listen to OnTabSelected; Next and Previous wrap around.
UCLASS(Abstract)
class UFVTabBarWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UPanelWidget TabPanel;

    UPROPERTY(EditAnywhere, Category = "Tabs")
    TSubclassOf<UFVTabButtonWidget> TabButtonClass;

    UPROPERTY()
    FFVOnTabSelected OnTabSelected;

    private TArray<UFVTabButtonWidget> Buttons;
    private int SelectedIndex = -1;

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SetTabs(TArray<FText> Labels)
    {
        TabPanel.ClearChildren();
        Buttons.Empty();
        SelectedIndex = -1;

        if (!TabButtonClass.IsValid())
        {
            Print(f"{GetName()}: Tab Button Class is not set.");
            return;
        }

        for (int i = 0; i < Labels.Num(); i++)
        {
            UFVTabButtonWidget Button = WidgetBlueprint::CreateWidget(TabButtonClass, GetOwningPlayer());
            Button.Setup(Labels[i], i);
            Button.OnTabClicked.AddUFunction(this, n"HandleTabClicked");
            TabPanel.AddChild(Button);
            Buttons.Add(Button);
        }

        SetVisibility(Labels.Num() > 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectTab(int Index)
    {
        if (!Buttons.IsValidIndex(Index) || Index == SelectedIndex)
        {
            return;
        }

        SelectedIndex = Index;
        for (int i = 0; i < Buttons.Num(); i++)
        {
            Buttons[i].SetSelected(i == SelectedIndex);
        }
        OnTabSelected.Broadcast(SelectedIndex);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectNext()
    {
        if (Buttons.Num() > 0)
        {
            SelectTab((SelectedIndex + 1) % Buttons.Num());
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void SelectPrevious()
    {
        if (Buttons.Num() > 0)
        {
            SelectTab((SelectedIndex - 1 + Buttons.Num()) % Buttons.Num());
        }
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int GetSelectedIndex() const
    {
        return SelectedIndex;
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    int GetTabCount() const
    {
        return Buttons.Num();
    }

    UFUNCTION()
    private void HandleTabClicked(int Index)
    {
        SelectTab(Index);
    }
}
