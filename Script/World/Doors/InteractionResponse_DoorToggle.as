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

    // The toggle isn't told who uses it, so the interaction in progress is tracked for keys.
    private UFVInteractorComponent CurrentInteractor;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        OnTimelineCallback.BindUFunction(this, n"OnTimelineProgress");

        UFVInteractableComponent Interactable = UFVInteractableComponent::Get(GetOwner());
        if (IsValid(Interactable))
        {
            Interactable.InteractionStarted.AddUFunction(this, n"OnInteractionStarted");
            Interactable.InteractionEnded.AddUFunction(this, n"OnInteractionEnded");
        }
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
        if (TryOpenWithKey())
        {
            return;
        }

        UTimelineComponent Timeline = GetOwner().GetComponentByClass(UTimelineComponent);
        if (!IsValid(Timeline))
        {
            return;
        }

        Timeline.AddInterpFloat(LockedShakeCurve, OnTimelineCallback);
        Timeline.PlayFromStart();
	}

    // Unlocks with a key the interactor carries, then runs the action again now that the lock no longer blocks it.
    private bool TryOpenWithKey()
    {
        UFVInteractableComponent Interactable = UFVInteractableComponent::Get(GetOwner());

        if (!IsValid(CurrentInteractor) || !IsValid(Interactable)
            || !FVItemKeys::TryUnlockWithKey(CurrentInteractor.GetOwner(), UFVLockComponent::Get(GetOwner())))
        {
            return false;
        }

        Interactable.ExecuteAction(GetActionTag(), CurrentInteractor);
        return true;
    }

    UFUNCTION()
    private void OnInteractionStarted(const FGameplayTag&in ActionTag, UFVInteractorComponent Interactor)
    {
        CurrentInteractor = Interactor;
    }

    UFUNCTION()
    private void OnInteractionEnded(const FGameplayTag&in ActionTag, UFVInteractorComponent Interactor, bool bSuccess)
    {
        CurrentInteractor = nullptr;
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