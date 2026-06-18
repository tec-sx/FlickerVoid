class AFVPlayerCharacter : AFVCharacter
{
    UPROPERTY(DefaultComponent, Category = "Movement")
    UMotionWarpingComponent MotionWarpingComponent;

    UPROPERTY(DefaultComponent, Category = "Interaction")
    UFVInteractionInstigatorComponent InteractionComponent;

    UPROPERTY(DefaultComponent, Category = "Animation")
    UContextualAnimSceneActorComponent ContextualAnimation;

    UPROPERTY()
    UFVTraversalComponent Traversal;

    bool bJustLanded = false;
    bool bIsRagdolling = false;

    AFVPlayerCharacter()
    {
        bUseControllerRotationPitch = false;
        bUseControllerRotationYaw = false;
        bUseControllerRotationRoll = false;
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        Traversal = UFVTraversalComponent::Get(this);

        // Configure initial movement settings
        CharacterMovement.bOrientRotationToMovement = false;
        CharacterMovement.bUseControllerDesiredRotation = true;
    }

    UFUNCTION(BlueprintOverride)
    bool RequestTraverse()
    {
        FFVTraversalCharacterData CharacterData;
        CharacterData.CapsuleRadius = CapsuleComponent.CapsuleRadius;
        CharacterData.CapsuleHalfHeight = CapsuleComponent.CapsuleHalfHeight;
        CharacterData.ForwardDirection = GetActorForwardVector();
        CharacterData.Location = GetActorLocation();
        CharacterData.Rotation = GetActorRotation();
        CharacterData.Velocity = Velocity;
        CharacterData.Speed = Velocity.Size2D();
        CharacterData.MovementMode = CharacterMovement.MovementMode;
        CharacterData.Mesh = Mesh;
        CharacterData.MotionWarping = MotionWarpingComponent;

        EDrawDebugTrace DrawDebugType = CharacterMovement.IsMovingOnGround()
            ? EDrawDebugTrace::ForOneFrame
            : EDrawDebugTrace::ForDuration;

        bool bIsTraversing = Traversal.TryTraversalAction(CharacterData, DrawDebugType);

        SetTraversing(bIsTraversing);

        return bIsTraversing;
    }

    UFUNCTION(BlueprintOverride)
    void OnWalkingOffLedge(
		FVector PreviousFloorImpactNormal,
		FVector PreviousFloorContactNormal,
        FVector PreviousLocation,
		float TimeDelta)
    {
        UnCrouch();
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
}