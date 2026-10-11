// Main menu, pushed on UI.Layer.Menu by AFVMainMenuHUD: Play (Continue, New Game, Load), Options and Quit.
// Continue and Load stay disabled until loading a save can also travel to its map.
UCLASS(Abstract)
class UFVMainMenuWidget : UFVActivatableWidget
{
    default InputMode = EFVWidgetInputMode::Menu;
    default bIsBackHandler = true;
    default bIsFocusable = true;

    UPROPERTY(EditDefaultsOnly, Category = "Main Menu")
    TSoftObjectPtr<UWorld> NewGameLevel;

    UPROPERTY(EditDefaultsOnly, Category = "Main Menu")
    TSubclassOf<UFVOptionsMenuWidget> OptionsClass;

    UPROPERTY(BindWidget)
    UWidgetSwitcher MenuSwitcher;

    UPROPERTY(BindWidget)
    UButton PlayButton;

    UPROPERTY(BindWidget)
    UButton OptionsButton;

    UPROPERTY(BindWidget)
    UButton QuitButton;

    UPROPERTY(BindWidget)
    UButton ContinueButton;

    UPROPERTY(BindWidget)
    UButton NewGameButton;

    UPROPERTY(BindWidget)
    UButton LoadButton;

    UPROPERTY(BindWidgetOptional)
    UButton BackButton;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        PlayButton.OnClicked.AddUFunction(this, n"HandlePlayClicked");
        OptionsButton.OnClicked.AddUFunction(this, n"HandleOptionsClicked");
        QuitButton.OnClicked.AddUFunction(this, n"HandleQuitClicked");
        ContinueButton.OnClicked.AddUFunction(this, n"HandleContinueClicked");
        NewGameButton.OnClicked.AddUFunction(this, n"HandleNewGameClicked");
        LoadButton.OnClicked.AddUFunction(this, n"HandleLoadClicked");
        if (BackButton != nullptr)
        {
            BackButton.OnClicked.AddUFunction(this, n"HandleBackClicked");
        }

        ContinueButton.SetIsEnabled(false);
        LoadButton.SetIsEnabled(false);
    }

    UFUNCTION(BlueprintOverride)
    void OnActivated()
    {
        ShowPanel(0);
    }

    // Back from the Play panel returns to the root; the root itself stays.
    UFUNCTION(BlueprintOverride)
    bool OnHandleBackAction()
    {
        if (MenuSwitcher.GetActiveWidgetIndex() != 0)
        {
            ShowPanel(0);
        }
        return true;
    }

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        return MenuSwitcher.GetActiveWidgetIndex() == 0 ? PlayButton : NewGameButton;
    }

    private void ShowPanel(int Index)
    {
        MenuSwitcher.SetActiveWidgetIndex(Index);
        RequestRefreshFocus();
    }

    UFUNCTION()
    private void HandlePlayClicked()
    {
        ShowPanel(1);
    }

    UFUNCTION()
    private void HandleBackClicked()
    {
        ShowPanel(0);
    }

    UFUNCTION()
    private void HandleOptionsClicked()
    {
        UFVUIManagerSubsystem UI = UFVUIManagerSubsystem::Get(GetOwningPlayer());
        if (UI != nullptr && OptionsClass.IsValid())
        {
            UI.PushScreen(GameplayTags::UI_Layer_Menu, OptionsClass);
        }
    }

    UFUNCTION()
    private void HandleQuitClicked()
    {
        System::QuitGame(GetOwningPlayer(), EQuitPreference::Quit, false);
    }

    UFUNCTION()
    private void HandleContinueClicked()
    {
    }

    UFUNCTION()
    private void HandleNewGameClicked()
    {
        if (NewGameLevel.IsNull())
        {
            Print(f"{GetName()}: New Game Level is not set.");
            return;
        }
        Gameplay::OpenLevelBySoftObjectPtr(NewGameLevel);
    }

    UFUNCTION()
    private void HandleLoadClicked()
    {
    }
}
