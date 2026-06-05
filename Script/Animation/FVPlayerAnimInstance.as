class UFVPlayerAnimInstance : UFVCharacterAnimInstance
{
    UPROPERTY()
    FTransform InteractionTransform;

    UPROPERTY()
    float LocomotionDirectionAngle;

    // ========================================================================
	// ESSENTIAL VALUES
	// ========================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FTransform RootTransform;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector RelativeAcceleration;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector VelocityAcceleration;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector LastNonZeroVelocity;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float GroundSpeed;

    // ========================================================================
	// STATES
	// ========================================================================
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) EFVRotationMode RotationMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) EFVRotationMode RotationModeLastFrame;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite) EFVMovementHipDirection HipDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) EFVMovementHipDirection HipDirectionLastFrame;

    // ========================================================================
	// TRAJECTORY
	// ========================================================================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FPoseSearchTrajectoryData TrajectoryIdleGenerationData;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FPoseSearchTrajectoryData TrajectoryMovingGenerationData;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FTransformTrajectory Trajectory_;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FPoseSearchTrajectory_WorldCollisionResults Collision;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float PreviousDesiredYaw;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector TrajectoryPastVelocity;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector TrajectoryCurrentVelocity;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector TrajectoryFutureVelocity;

    UFUNCTION(BlueprintOverride)
    void BlueprintThreadSafeUpdateAnimation(float DeltaTime)
    {
    }

    UFUNCTION()
    void SetInteractionTransform(FTransform InTransform)
    {
        InteractionTransform = InTransform;
    }

    UFUNCTION(BlueprintEvent)
    FPoseHistoryReference GetPoseHistory()
    {
        return FPoseHistoryReference();
    }

    // UFUNCTION()
    // FTransform GetInteractionTransform()
    // {
    //     return InteractionTransform;
    // }
};