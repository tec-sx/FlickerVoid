// The always-on HUD, pushed once on UI.Layer.HUD. Three areas laid out in its Blueprint: Information (quest tracker,
// notifications, player status), Action (scanner readout, context hints; the interaction prompt draws on UI.Layer.Game)
// and Navigation (minimap with ring compass). Hides itself while a screen is open on UI.Layer.GameMenu.
UCLASS(Abstract)
class UFVGameHUDWidget : UFVActivatableWidget
{
    default InputMode = EFVWidgetInputMode::Game;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    bool bHideWhileGameMenuOpen = true;

    UPROPERTY(BindWidgetOptional)
    UWidget InformationArea;

    UPROPERTY(BindWidgetOptional)
    UWidget ActionArea;

    UPROPERTY(BindWidgetOptional)
    UWidget NavigationArea;

    private bool bHiddenForMenu = false;

    UFUNCTION(BlueprintOverride)
    void Tick(FGeometry MyGeometry, float InDeltaTime)
    {
        if (!bHideWhileGameMenuOpen)
        {
            return;
        }

        bool bMenuOpen = IsGameMenuOpen();
        if (bMenuOpen != bHiddenForMenu)
        {
            bHiddenForMenu = bMenuOpen;
            SetAreasVisible(!bMenuOpen);
        }
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    bool IsGameMenuOpen() const
    {
        UFVUIManagerSubsystem UI = UFVUIManagerSubsystem::Get(GetOwningPlayer());
        UFVUILayout Layout = UI != nullptr ? UI.GetLayout() : nullptr;
        UCommonActivatableWidgetContainerBase Layer = Layout != nullptr ? Layout.GetLayer(GameplayTags::UI_Layer_GameMenu) : nullptr;
        return Layer != nullptr && Layer.GetActiveWidget() != nullptr;
    }

    // Collapses the areas rather than the whole widget so it keeps ticking.
    private void SetAreasVisible(bool bVisible)
    {
        ESlateVisibility AreaVisibility = bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
        if (InformationArea != nullptr)
        {
            InformationArea.SetVisibility(AreaVisibility);
        }
        if (ActionArea != nullptr)
        {
            ActionArea.SetVisibility(AreaVisibility);
        }
        if (NavigationArea != nullptr)
        {
            NavigationArea.SetVisibility(AreaVisibility);
        }
        OnGameMenuVisibilityChanged(!bVisible);
    }

    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnGameMenuVisibilityChanged(bool bGameMenuOpen) {}
}
