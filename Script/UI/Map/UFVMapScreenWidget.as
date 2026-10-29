// The world map screen (M), pushed on UI.Layer.GameMenu outside the hub. Wraps a world map widget from
// FVNavigationSystem, opens on the player's map centred on them, and closes on Back or the map key.
UCLASS(Abstract)
class UFVMapScreenWidget : UFVScreenWidget
{
    UPROPERTY(BindWidget)
    UFVWorldMapWidget WorldMap;

    UPROPERTY(BindWidgetOptional)
    UTextBlock MapNameText;

    UFUNCTION(BlueprintOverride)
    void OnActivated()
    {
        WorldMap.ShowMap(nullptr, -1);
        WorldMap.CenterOnPlayer();

        if (MapNameText != nullptr)
        {
            UFVMapDefinition Map = WorldMap.GetShownMap();
            MapNameText.SetText(Map != nullptr ? Map.Display.Name : FText());
        }
    }

    UFUNCTION(BlueprintOverride)
    UWidget BP_GetDesiredFocusTarget() const
    {
        return WorldMap;
    }
}
