// The player's controller: navigation for the HUD maps, and the UI keys. The input config's actions tagged
// InputTag.UI.* arrive in OnUIInputPressed: HubMenu toggles the hub menu and Map the world map (both on
// UI.Layer.GameMenu), Pause opens the pause menu (UI.Layer.Menu). Open screens take menu input only and close themselves (UFVScreenWidget).
class APlayerCharacterController : AFVPlayerController
{
    UPROPERTY(DefaultComponent)
    UFVNavigatorComponent NavigatorComponent;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UFVHubMenuWidget> HubMenuClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UFVMapScreenWidget> MapScreenClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UFVPauseMenuWidget> PauseMenuClass;

    private UCommonActivatableWidget HubMenu;
    private UCommonActivatableWidget MapScreen;
    private UCommonActivatableWidget PauseMenu;

    UFUNCTION(BlueprintOverride)
    void OnUIInputPressed(FGameplayTag InputTag)
    {
        if (InputTag == GameplayTags::InputTag_UI_HubMenu)
        {
            HubMenu = Toggle(HubMenu, GameplayTags::UI_Layer_GameMenu, HubMenuClass);
        }
        else if (InputTag == GameplayTags::InputTag_UI_Map)
        {
            MapScreen = Toggle(MapScreen, GameplayTags::UI_Layer_GameMenu, MapScreenClass);
        }
        else if (InputTag == GameplayTags::InputTag_UI_Pause && !IsOpen(PauseMenu))
        {
            PauseMenu = Push(GameplayTags::UI_Layer_Menu, PauseMenuClass);
        }
    }

    // Opens the hub on a given page, e.g. the journal from a quest notification.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void OpenHubMenu(TSubclassOf<UFVHubMenuPageWidget> Page)
    {
        if (!IsOpen(HubMenu))
        {
            HubMenu = Push(GameplayTags::UI_Layer_GameMenu, HubMenuClass);
        }

        UFVHubMenuWidget Hub = Cast<UFVHubMenuWidget>(HubMenu);
        if (Hub != nullptr && Page.IsValid())
        {
            Hub.OpenPage(Page);
        }
    }

    private UCommonActivatableWidget Toggle(UCommonActivatableWidget Current, FGameplayTag Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass)
    {
        if (IsOpen(Current))
        {
            UFVUIManagerSubsystem::Get(this).PopScreen(Current);
            return nullptr;
        }
        return Push(Layer, WidgetClass);
    }

    private UCommonActivatableWidget Push(FGameplayTag Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass)
    {
        UFVUIManagerSubsystem UI = UFVUIManagerSubsystem::Get(this);
        if (UI == nullptr || !WidgetClass.IsValid())
        {
            Print(f"{GetName()}: no UI manager or screen class for {Layer}.");
            return nullptr;
        }
        return UI.PushScreen(Layer, WidgetClass);
    }

    private bool IsOpen(UCommonActivatableWidget Widget) const
    {
        return Widget != nullptr && Widget.IsActivated();
    }
}
