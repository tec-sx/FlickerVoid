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
    UInteractableComponent Interactable;
    default Interactable.Type = GameplayTags::Interactable_Door;
    default Interactable.DetectionRadius = 160.f;
    default Interactable.FocusComponentTag = n"Interactable";

    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent)
    UTimelineComponent Timeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    UCurveFloat OpenCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    UCurveFloat LockedShakeCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsLocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenAngle = 90.0f;

    private FOnTimelineFloat OnTimelineCallback;
    private TArray<UDoorRootComponent> Doors;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        Interactable.OnInteractionExecuted.AddUFunction(this, n"OnInteractionExecuted");
        OnTimelineCallback.BindUFunction(this, n"HandleTimelineProgress");

        Doors.Empty();
        Doors = GetComponentsByClass(UDoorRootComponent);
    }

    UFUNCTION()
    private void OnInteractionExecuted(const FGameplayTag&in ActionTag, UInteractorComponent Interactor)
    {
        if (ActionTag == GameplayTags::Interaction_Action_Open)
        {
            if (bIsLocked)
            {
                Timeline.AddInterpFloat(LockedShakeCurve, OnTimelineCallback);
                Timeline.PlayFromStart();
                
                return;
            }

            Timeline.AddInterpFloat(OpenCurve, OnTimelineCallback);

            if (bIsOpen)
            {
                Timeline.SetPlaybackPosition(0.6f, true);
                Timeline.Reverse();
                bIsOpen = false;
            }
            else
            {
                Timeline.PlayFromStart();
                bIsOpen = true;
            }
        }
    }

    UFUNCTION()
    private void HandleTimelineProgress(float32 InterpValue)
    {
        for (UDoorRootComponent Door : Doors)
        {
            float Direction = Door.bReverseDirection ? -1 : 1;
            FRotator FinalValue = Door.Direction * InterpValue * OpenAngle * Direction;

            Door.SetRelativeRotation(FinalValue);
        }
    }
}