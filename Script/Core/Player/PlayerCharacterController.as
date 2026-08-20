class APlayerCharacterController : AFVPlayerController
{
    UPROPERTY(DefaultComponent)
    UNarrativeNavigationComponent Navigation;
    
    UPROPERTY()
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY()
    UUserWidget HUDWidget;

    UFUNCTION(BlueprintOverride)
    void ActorOnClicked(FKey ButtonPressed)
    {
        if (ButtonPressed == FKey(n"M"))
        {
            Print("M");
        }
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        HUDWidget = WidgetBlueprint::CreateWidget(HUDWidgetClass, this);
        HUDWidget.AddToViewport();
    }
}