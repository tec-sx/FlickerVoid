class AFVPlayerCharacter : AFVCharacter
{
    UPROPERTY(DefaultComponent, Category = "Movement")
    UMotionWarpingComponent MotionWarpingComponent;

    bool bJustanded = false;
    bool bIsRagdolling = false;
    
    FFVAnimaData AnimData;

    UFUNCTION(BlueprintOverride)
    void OnLaunched(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
    {
    }

    AFVPlayerCharacter()
    {
        bUseControllerRotationPitch = false;
        bUseControllerRotationYaw = false; 
        bUseControllerRotationRoll = false;
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        // Configure initial movement settings
        CharacterMovement.bOrientRotationToMovement = false;
        CharacterMovement.bUseControllerDesiredRotation = true;
    }

    UFUNCTION(BlueprintOverride)
    void OnJumped()
    {
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
        Settings.PlayerGait = EFVGait::Walk;
        Settings.PlayerStance = EFVStance::Stand;

        return Settings;
    }

    UFUNCTION()
    FFVTraversalCharacterData CreateTraversalData()
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
        return CharacterData;
    }
}