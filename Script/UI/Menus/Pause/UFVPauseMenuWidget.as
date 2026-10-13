// Pause menu (Esc), pushed on UI.Layer.Menu: Resume, Options and Quit to the main menu. Pauses the game while open;
// Options opens on top of it without resuming.
UCLASS(Abstract)
class UFVPauseMenuWidget : UFVScreenWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Pause Menu")
    TSoftObjectPtr<UWorld> MainMenuLevel;

    UPROPERTY(EditDefaultsOnly, Category = "Pause Menu")
    TSubclassOf<UFVOptionsMenuWidget> OptionsClass;

    UPROPERTY(BindWidget)
    UButton ResumeButton;

    UPROPERTY(BindWidget)
    UButton OptionsButton;

    UPROPERTY(BindWidget)
    UButton QuitButton;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        Super::OnInitialized();
        ResumeButton.OnClicked.AddUFunction(this, n"HandleResumeClicked");
        OptionsButton.OnClicked.AddUFunction(this, n"HandleOptionsClicked");
        QuitButton.OnClicked.AddUFunction(this, n"HandleQuitClicked");
    }

    UFUNCTION(BlueprintOverride)
    void OnActivated()
    {
        Gameplay::SetGamePaused(true);
    }

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        return ResumeButton;
    }

    void OnClosing() override
    {
        Gameplay::SetGamePaused(false);
    }

    UFUNCTION()
    private void HandleResumeClicked()
    {
        Close();
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
        Gameplay::SetGamePaused(false);
        if (MainMenuLevel.IsNull())
        {
            Print(f"{GetName()}: Main Menu Level is not set.");
            return;
        }
        Gameplay::OpenLevelBySoftObjectPtr(MainMenuLevel);
    }
}
