class UInteractionResponse_DoorToggle : UFVInteractableResponseComponent_Toggle
{   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    UCurveFloat OpenCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    UCurveFloat LockedShakeCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenAngle = 90.0f;

    TArray<UDoorRootComponent> Doors;
    FOnTimelineFloat OnTimelineCallback;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        OnTimelineCallback.BindUFunction(this, n"OnTimelineProgress");
    }

    UFUNCTION(BlueprintOverride)
	void OnToggled(bool bIsOpen)
	{
        UTimelineComponent Timeline = GetOwner().GetComponentByClass(UTimelineComponent);
        if (!IsValid(Timeline))
        {
            return;
        }

        Timeline.AddInterpFloat(OpenCurve, OnTimelineCallback);

        if (!bIsOpen)
        {
            Timeline.SetPlaybackPosition(0.6f, true);
            Timeline.Reverse();
        }
        else
        {
            Timeline.PlayFromStart();
        }
	}

	UFUNCTION(BlueprintOverride)
	void OnLockChanged(bool bLocked)
	{
	}

	UFUNCTION(BlueprintOverride)
	void OnToggleBlocked(bool bOpen)
	{
        UTimelineComponent Timeline = GetOwner().GetComponentByClass(UTimelineComponent);
        if (!IsValid(Timeline))
        {
            return;
        }

        Timeline.AddInterpFloat(LockedShakeCurve, OnTimelineCallback);
        Timeline.PlayFromStart();
	}

    UFUNCTION()
    private void OnTimelineProgress(float32 InterpValue)
    {
        for (UDoorRootComponent Door : Doors)
        {
            float Direction = Door.bReverseDirection ? -1 : 1;
            FRotator FinalValue = Door.Direction * InterpValue * OpenAngle * Direction;

            Door.SetRelativeRotation(FinalValue);
        }
    }
}