class UDoorRootComponent : USceneComponent
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bReverseDirection = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    FRotator Direction = FRotator(0, 1, 0);
}

class ADoor : AActor
{
    UPROPERTY(DefaultComponent)
    UInteractionResponse_DoorToggle DoorToggleComponent;
    default DoorToggleComponent.bStartOpen = false;

    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;
    
    UPROPERTY(DefaultComponent)
    UTimelineComponent Timeline;

    UPROPERTY(DefaultComponent)
    UFVInteractableComponent Interactable;

    UPROPERTY(DefaultComponent)
    UFVInteractableHighlightComponent Highlight;
    default Highlight.HighlightSetup.HighlightType = EFVHighlightType::PostProcessing;
    default Highlight.HighlightSetup.StencilID = 133;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        DoorToggleComponent.Doors.Empty();
        DoorToggleComponent.Doors = GetComponentsByClass(UDoorRootComponent);

        Interactable.BindResponse(GameplayTags::Interaction_Action_Open, DoorToggleComponent);
    }
}