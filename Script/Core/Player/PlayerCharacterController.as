class APlayerCharacterController : AFVPlayerController
{
    UPROPERTY(DefaultComponent)
    UFVNavigatorComponent NavigatorComponent;
    
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