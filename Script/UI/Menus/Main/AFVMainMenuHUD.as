// HUD for the main menu level: pushes the main menu on UI.Layer.Menu once the layout exists.
class AFVMainMenuHUD : AFVHUD
{
    UPROPERTY(EditDefaultsOnly, Category = "Main Menu")
    TSubclassOf<UFVMainMenuWidget> MainMenuClass;

    UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ToolTip = "Its saved volume and subtitle options are applied when play starts."))
    TSubclassOf<UFVOptionsMenuWidget> OptionsClass;

    UFUNCTION(BlueprintOverride)
    void OnLayoutReady(UFVUILayout NewLayout)
    {
        FVOptions::ApplySaved(OptionsClass);

        if (!MainMenuClass.IsValid())
        {
            Print(f"{GetName()}: Main Menu Class is not set.");
            return;
        }

        UFVUIManagerSubsystem UI = UFVUIManagerSubsystem::Get(GetOwningPlayerController());
        if (UI != nullptr)
        {
            UI.PushScreen(GameplayTags::UI_Layer_Menu, MainMenuClass);
        }
    }
}
