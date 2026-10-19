// The game's HUD actor: once the framework layout exists, pushes the always-on HUD widget on UI.Layer.HUD.
// Set a Blueprint of it as the game mode's HUD class.
class AFVGameHUD : AFVHUD
{
    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<UFVGameHUDWidget> HUDWidgetClass;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "HUD")
    UCommonActivatableWidget HUDWidget;

    UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ToolTip = "Its saved volume and subtitle options are applied when play starts."))
    TSubclassOf<UFVOptionsMenuWidget> OptionsClass;

    UFUNCTION(BlueprintOverride)
    void OnLayoutReady(UFVUILayout NewLayout)
    {
        FVOptions::ApplySaved(OptionsClass);

        if (!HUDWidgetClass.IsValid())
        {
            Print(f"{GetName()}: HUD Widget Class is not set.");
            return;
        }

        UFVUIManagerSubsystem UI = UFVUIManagerSubsystem::Get(GetOwningPlayerController());
        if (UI != nullptr)
        {
            HUDWidget = UI.PushScreen(GameplayTags::UI_Layer_HUD, HUDWidgetClass);
        }
    }
}
