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
    UFVInteractableComponent Interactable;
    default Interactable.Type = GameplayTags::Interactable_Door;
    default Interactable.DetectionRadius = 160.f;
    default Interactable.FocusComponentTag = n"Interactable";

    UPROPERTY(DefaultComponent)
    UInteractionResponse_DoorToggle DoorToggleComponent;
    default DoorToggleComponent.bStartOpen = false;

    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;
    
    UPROPERTY(DefaultComponent)
    UTimelineComponent Timeline;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        DoorToggleComponent.Doors.Empty();
        DoorToggleComponent.Doors = GetComponentsByClass(UDoorRootComponent);
    }
}