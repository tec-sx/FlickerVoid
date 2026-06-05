class UFVGroundMovementHandler : UFVMovementHandlerBase
{
    UPROPERTY(Category = "Configuration")
    UFVGroundMovementConfig GroundConfig;

    UFUNCTION(BlueprintOverride)
    void OnInitialize(AFVCharacter InCharacter,
					  UFVCharacterMovementComponent InMovementComponent,
                      FFVMovementHandlerInfo InConfig)
    {
        GroundConfig = Cast<UFVGroundMovementConfig>(InConfig.HandlerConfig);

        FFVResult Result = FVValidation::ValidateObject(GroundConfig, "GroundConfig");
        
        if (!Result.bSuccess)
        {
            Warning(Result.ErrorMessage + " on " + GetName() + ". Using default values.");
            GroundConfig = NewObject(this, UFVGroundMovementConfig);
        }

        MovementComponent.SprintAngleThreshold = GroundConfig.SprintAngleThreshold;
    }

    UFUNCTION(BlueprintOverride)
    bool CanEnter()
    {

    }

	UFUNCTION(BlueprintOverride)
	void GenerateMovement(float DeltaTime)
	{
        // Update Rotation Data
        {
            if (Character.HasTag(GameplayTags::Character_Action_Aim) || Character.MovementDirection.Y != 0.f)
            {
                // MovementComponent.bUseControllerDesiredRotation = true;
                // MovementComponent.bOrientRotationToMovement = false;
            }
            else
            {
                MovementComponent.bUseControllerDesiredRotation = false;
                MovementComponent.bOrientRotationToMovement = true;
            }
        }

        // Update Movement Data
        {
            FFVGaitConfig GaitConfig = GetGaitConfig();
            
            MovementComponent.MaxAcceleration = CalculateMaxAcceleration(GaitConfig);
            MovementComponent.BrakingDecelerationWalking = CalculateBrakingDeceleration();
            MovementComponent.GroundFriction = CalculateGroundFriction(GaitConfig);

            MovementComponent.MaxWalkSpeed = CalculateDirectionalSpeed(GaitConfig.Speeds);
            MovementComponent.MaxWalkSpeedCrouched = CalculateDirectionalSpeed(GroundConfig.CrouchSpeeds);
        }
 
        // Perform Movement if there is movement input
        if (Character.MovementDirection.Size() > 0)
        {
            FRotator ControlYaw = FRotator(0.f, Character.ControlRotation.Yaw, 0.f);

            Character.AddMovementInput(ControlYaw.ForwardVector, Character.MovementDirection.X);
            Character.AddMovementInput(ControlYaw.RightVector, Character.MovementDirection.Y);
        }

        if (Character.HasTag(GameplayTags::Movement_Action_Jump))
        {
            Character.Jump();
            Character.RemoveTag(GameplayTags::Movement_Action_Jump);
        }
	}
    
     FFVGaitConfig GetGaitConfig()
    {
        if (Character.HasTag(GameplayTags::Movement_Gait_Walking))
            return GroundConfig.WalkConfig;
        else if (Character.HasTag(GameplayTags::Movement_Gait_Jogging))
            return GroundConfig.JogConfig;
        else if (Character.HasTag(GameplayTags::Movement_Gait_Sprinting))
            return GroundConfig.SprintConfig;
        else
            return GroundConfig.WalkConfig;
    }
    

    float CalculateMaxAcceleration(FFVGaitConfig GaitConfig)
    {
        if (Character.HasTag(GameplayTags::Movement_Gait_Walking) ||
			Character.HasTag(GameplayTags::Movement_Gait_Jogging))
             return GaitConfig.Acceleration;
        else if (Character.HasTag(GameplayTags::Movement_Gait_Sprinting))
            return Math::GetMappedRangeValueClamped(
                        GaitConfig.SpeedRangeForAcceleration,
                        GaitConfig.AccelerationRange,
                        MovementComponent.Velocity.Size2D());
        else
            return 300.f;
    }

    float CalculateBrakingDeceleration()
    {
        return HasMovementInput() ? GroundConfig.BrakingWithInput : GroundConfig.BrakingWithoutInput;
    }

    float CalculateGroundFriction(FFVGaitConfig GaitConfig)
    { 
        if (Character.HasTag(GameplayTags::Movement_Gait_Walking) ||
			Character.HasTag(GameplayTags::Movement_Gait_Jogging))
                return GaitConfig.Friction;
        else if (Character.HasTag(GameplayTags::Movement_Gait_Sprinting))
            return Math::GetMappedRangeValueClamped(
                        GaitConfig.SpeedRangeForFriction,
                        GaitConfig.FrictionRange,
                        MovementComponent.Velocity.Size2D());
        else
            return 8.f;
    }

    float CalculateDirectionalSpeed(FVector SpeedRange)
    {
        float StrafeSpeedMap = GetStrafeSpeedMapCurveValue();

        //  0 = forward, 1 = strafe L or R, 2 = Backwards.
        if (StrafeSpeedMap < 1.f)
        {
            return Math::GetMappedRangeValueClamped(
                FVector2D(0.f, 1.f),
                FVector2D(SpeedRange.X, SpeedRange.Y),
                StrafeSpeedMap);
        }
        else
        {
            return Math::GetMappedRangeValueClamped(
                FVector2D(1.f, 2.f),
                FVector2D(SpeedRange.Y, SpeedRange.Z),
                StrafeSpeedMap);
        }
    }

    bool HasMovementInput()
    {
        return !Character.GetPendingMovementInputVector().IsNearlyZero();
    }

    float GetStrafeSpeedMapCurveValue()
    {
        float StrafeSpeedMap = 0.f;

        if(MovementComponent.bUseControllerDesiredRotation)
        {
            if (GroundConfig.StrafeSpeedMapCurve == nullptr)
            {
                // Warning already logged in GenerateMovement, just return default
                return 0.f;
            }

            float Angle = AnimGraph::CalculateDirection(
                MovementComponent.Velocity,
                Character.GetActorRotation());

            Angle = Math::Abs(Angle);

            StrafeSpeedMap = GroundConfig.StrafeSpeedMapCurve.GetFloatValue(Angle);
        }
        
        return StrafeSpeedMap;
    }
};