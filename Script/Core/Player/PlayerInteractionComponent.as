const FConsoleVariable CVar_InteractionDebug("FVCvar.Interaction.Debug", false, "Debug the interaction system");

class UPlayerInteractionComponent : UActorComponent
{
    UPROPERTY(Transient)
    private TArray<FInteractionTarget> AvailableTargets;

    private bool bIsFocused;
    private bool bInteractionIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "-1", ClampMax = "1"), Category=Interaction)
	float DetectionConeAngle = 0.5f;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        UInteractionBus InteractionBus = UInteractionBus::Get();
        // InteractionBus.OnInteractionZoneEvent.AddUFunction(this, n"InteractionZoneEventHandler");

        SetComponentTickEnabled(false);
        SetComponentTickInterval(0.08f);

        AvailableTargets.Reserve(16);
    }

    UFUNCTION()
    private void InteractionZoneEventHandler(FGameplayTag EventType, FInteractionTarget Payload)
    {
        if (EventType.IsValid() && Payload.IsValid())
        {
            if (EventType == GameplayTags::Interaction_Event_InteractionZoneEnter)
            {
                if (!AvailableTargets.Contains(Payload))
                {
                    AvailableTargets.Add(Payload);
                }
            }
            
            if (EventType == GameplayTags::Interaction_Event_InteractionZoneExit)
            {
                if (AvailableTargets.Contains(Payload))
                {
                    AvailableTargets.Remove(Payload);
                }
            }

            UpdateInteractionState();
        }

    }

    UFUNCTION(BlueprintOverride)
    void Tick(float DeltaSeconds)
    {
        FInteractionTarget FocusedTarget = FindBestTarget();

        if (FocusedTarget.IsValid())
        {
            // Print("Dialogue is active: " + bDialogueIsActive);
            // if (FocusedTarget.InteractableType == GameplayTags::Interactable_Character && !bDialogueIsActive)
            // {
            //     Print("Activating Dialogue");
            //     AInteractableAICharacter InteractableCharacter = Cast<AInteractableAICharacter>(FocusedTarget.Actor);
            //     if (IsValid(InteractableCharacter))
            //     {

            //     }
            // }
        }
    }

    private FInteractionTarget FindBestTarget()
    {
        // Determine the view direction — prefer controller if available
	    FVector ViewLocation = Owner.GetActorLocation();
	    FVector ViewForward = Owner.GetActorForwardVector();

        APawn PawnOwner = Cast<APawn>(GetOwner());

        if (IsValid(PawnOwner))
        {
            APlayerController PC = Cast<APlayerController>(PawnOwner.GetController());
            
            if (IsValid(PC))
            {
                FRotator ViewRotation;
                PC.GetPlayerViewPoint(ViewLocation, ViewRotation);
                ViewForward = ViewRotation.Vector();
            }
        }

        FInteractionTarget BestTarget;
        float BestScore = -1.f;

        for (FInteractionTarget CurrentTarget : AvailableTargets)
        {
            FVector TargetLocation = CurrentTarget.Actor.GetActorLocation() - ViewLocation;
            const float DotProduct = ViewForward.DotProduct(TargetLocation.GetSafeNormal());

            if (DotProduct < DetectionConeAngle)
            {
                continue;
            }

            const float DistanceScore = 1.0f / (1.0f + TargetLocation.Size());
            const float CurrentScore = DotProduct + DistanceScore;

            if (CurrentScore > BestScore)
            {
                BestScore = CurrentScore;
                BestTarget = CurrentTarget;
            }
        }

            FVector TargetOrigin = FVector::ZeroVector;
		    FVector TargetBoxExtent = FVector::ZeroVector;

            BestTarget.Actor.GetActorBounds(false, TargetOrigin, TargetBoxExtent);

        return BestTarget;
    }

    private void UpdateInteractionState()
    {
        if (AvailableTargets.Num() > 0)
        {
            AvailableTargets.Sort();
            SetComponentTickEnabled(true);
        }
        else
        {
            SetComponentTickEnabled(false);
        }
    }
}