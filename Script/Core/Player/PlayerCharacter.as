class APlayerCharacter : AFVPlayerCharacter
{
    UPROPERTY(DefaultComponent, Category = Movement)
    UMotionWarpingComponent MotionWarpingComponent;

    UPROPERTY(DefaultComponent, Category = Animation)
    UContextualAnimSceneActorComponent ContextualAnimation;

    UPROPERTY(DefaultComponent, Category = Camera)
    UGameplayCameraComponent GameplayCamera;

    UPROPERTY(DefaultComponent)
    UFVInteractionUIComponent InteractionUIComponent;
    
    UPROPERTY(DefaultComponent)
    UFVInventoryComponent InventoryComponent;

    UPROPERTY(DefaultComponent)
    UFVItemReceiverComponent ItemReceiverComponent;
    

    UPROPERTY(DefaultComponent, Category = Items)
    UFVHeldItemComponent HeldItems;

    UPROPERTY(DefaultComponent, Category = Items)
    UFVIntoxicationComponent Intoxication;

    UPROPERTY()
    UFVTraversalComponent Traversal;
    
    UPROPERTY()
    FIKGoal RightHandIKGoal;

    bool bIsRagdolling = false;

    default Mesh.SetRelativeLocation(FVector(0, 0, -CapsuleComponent.CapsuleHalfHeight));
    default Mesh.SetRelativeRotation(FRotator(0, -90, 0));

    UFUNCTION(BlueprintOverride)
    void Possessed(AController NewController)
    {
        APlayerCharacterController PlayerController = Cast<APlayerCharacterController>(NewController);

        if (IsValid(PlayerController))
        {
            GameplayCamera.AttachToComponent(
                Mesh,
                NAME_None,
                EAttachmentRule::SnapToTarget,
                EAttachmentRule::SnapToTarget,
                EAttachmentRule::KeepRelative,
                false);

            GameplayCamera.ActivateCameraForPlayerController(PlayerController, true);
        }

        FVAbilitySystemComponent.AddLooseGameplayTag(GameplayTags::Ability_Technical_LockPick);
    }

    APlayerCharacter()
    {
        bUseControllerRotationPitch = false;
        bUseControllerRotationYaw = false;
        bUseControllerRotationRoll = false;
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        HeldItems.OnHeldItemsChanged.AddUFunction(this, n"UpdateHoldPoses");
        UpdateHoldPoses();

        Traversal = UFVTraversalComponent::Get(this);

        Locomotion.OnStartedFalling.AddUFunction(this, n"OnStartedFalling");
    }

    UFUNCTION(BlueprintOverride)
    bool RequestTraverse()
    {
        EDrawDebugTrace DrawDebugType = Locomotion.IsOnGround()
            ? EDrawDebugTrace::ForOneFrame
            : EDrawDebugTrace::ForDuration;

        bool bIsTraversing = Traversal.TryTraversalAction(DrawDebugType);

        Locomotion.SetTraversing(bIsTraversing);

        return bIsTraversing;
    }

    UFUNCTION()
    void OnStartedFalling()
    {
        Locomotion.StopCrouching();
    }

    UFUNCTION()
    FFVPlayerCameraSettings GetCameraSettings()
    {
        FFVPlayerCameraSettings Settings;
        Settings.CameraMode = EFVCameraMode::Free;
        Settings.CameraStyle = EFVCameraStyle::Medium;
        Settings.PlayerGait = EFVGait::Walking;
        Settings.PlayerStance = EFVStance::Stand;

        return Settings;
    }

    UFUNCTION()
    bool HasFocusedInteractable()
    {
        return InteractorComponent.HasInteractableTarget();
    }

    // Passes the hold poses of what the hands hold to the anim instance.
    UFUNCTION()
    private void UpdateHoldPoses()
    {
        UFVPlayerAnimInstance AnimInstance = Cast<UFVPlayerAnimInstance>(Mesh.AnimInstance);

        if (AnimInstance != nullptr)
        {
            AnimInstance.RightHandHoldPose = HeldItems.GetHoldPose(true);
            AnimInstance.LeftHandHoldPose = HeldItems.GetHoldPose(false);
        }
    }
}