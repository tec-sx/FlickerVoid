struct FFVTraceParameters
{
    UPROPERTY()
    float TraceForwardDistance;

    UPROPERTY()
    FVector TraceOriginOffset;

    UPROPERTY()
    FVector TraceEndOffset;

    UPROPERTY()
    float TraceHalfHeight;

    UPROPERTY()
    float TraceRadius;
};

class UFV_ACTraversal : UActorComponent
{
    UPROPERTY()
    bool bIsTraversing;
    
    UPROPERTY(Category = "Configuration")
    UFVTraversalConfig Config;
    
    int DrawDebugLevel = 0;
    float DrawDebugDuration = 0;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        // Validate configuration
        if (Config == nullptr)
        {
            Warning("UFVTraversalConfig is null on " + GetOwner().GetName() + ". Using default values.");
            Config = NewObject(this, UFVTraversalConfig);
        }
    }

    UFUNCTION()
    bool TryTraversalAction(FFVTraversalCharacterData CharacterData, EDrawDebugTrace DrawDebugType)
    {
        DrawDebugLevel = System::GetConsoleVariableIntValue("FVCvar.Traversal.DrawDebugLevel");
        DrawDebugDuration = System::GetConsoleVariableFloatValue("FVCvar.Traversal.DrawDebugDuration");

        FFVTraceParameters Params = CalculateTraceParameters(CharacterData.MovementMode, CharacterData.Rotation, CharacterData.Velocity);
        EFVMovementMode TraversalMovementMode = GetTraversalMovementMode(CharacterData.MovementMode);

        FFVTraversalCheckResult TraverseResult;

        // Check if we hit a traversable obstacle
        bool bForwardTraceSuccess = DoForwardTrace(CharacterData, Params, DrawDebugType, TraverseResult);
        
        if (bForwardTraceSuccess == false)
        {
            return false;
        }

        // Check if character can fit over front and back ledges
        FHitResult BackLedgeClearanceHit;
        bool bClearanceSuccess = DoClearanceTraces(CharacterData, TraverseResult, DrawDebugType, BackLedgeClearanceHit);
        
        if (bClearanceSuccess == false)
        {
            return false;
        }

        // If there is room (no ceiling hit), calculate obstacle depth and check for back floor
        if (BackLedgeClearanceHit.bBlockingHit == false)
        {
            TraverseResult.ObstacleDepth = TraverseResult.FrontLedgeLocation.Distance(TraverseResult.BackLedgeLocation);
            TraverseResult.bHasBackLedge = true;

            DoFloorTrace(CharacterData, TraverseResult, DrawDebugType);
        }
        // If there is not room (ceiling hit), calculate obstacle depth from impact point and invalidate back ledge
        else
        {
            TraverseResult.ObstacleDepth = BackLedgeClearanceHit.ImpactPoint.Distance(TraverseResult.FrontLedgeLocation);
            TraverseResult.bHasBackLedge = false;
        }

        UFVPlayerAnimInstance PlayerAnimInstance = 
            Cast<UFVPlayerAnimInstance>(CharacterData.Mesh.GetAnimInstance());

        if (!FVValidation::ValidateObject(PlayerAnimInstance, "PlayerAnimInstance").bSuccess)
        {
            return false;
        }
        
        FTransform InteractTransform = 
            FTransform(FRotator::MakeFromZ(TraverseResult.FrontLedgeNormal), TraverseResult.FrontLedgeLocation);

        PlayerAnimInstance.SetInteractionTransform(InteractTransform);
        FPoseHistoryReference PoseHistory = PlayerAnimInstance.GetPoseHistory();

        FFVTraversalChooserInput ChooserInput;
        ChooserInput.ActionType = TraverseResult.ActionType;
        ChooserInput.bHasFrontLedge = TraverseResult.bHasFrontLedge;
        ChooserInput.bHasBackLedge = TraverseResult.bHasBackLedge;
        ChooserInput.bHasBackFloor = TraverseResult.bHasBackFloor;
        ChooserInput.ObstacleHeight = TraverseResult.ObstacleHeight;
        ChooserInput.ObstacleDepth = TraverseResult.ObstacleDepth;
        ChooserInput.BackLedgeHeight = TraverseResult.BackLedgeHeight;
        ChooserInput.DistanceToLedge = TraverseResult.FrontLedgeLocation.Distance(CharacterData.Location);
        ChooserInput.MovementMode = TraversalMovementMode;
        ChooserInput.Speed = CharacterData.Speed;
        ChooserInput.PoseHistory = PoseHistory;

        FFVTraversalChooserOutput ChooserOutput = EvaluateChooserTable(ChooserInput, PlayerAnimInstance);

        return false;
	}

    // Implement this in Blueprints.
    UFUNCTION(BlueprintEvent)
    FFVTraversalChooserOutput EvaluateChooserTable(FFVTraversalChooserInput Input, UAnimInstance AnimInstance)
    {
        return FFVTraversalChooserOutput();
    }

    bool DoForwardTrace(
		FFVTraversalCharacterData CharacterData,
		FFVTraceParameters Params,
		EDrawDebugTrace DrawDebugType,
		FFVTraversalCheckResult& TraverseResult)
    {
        FVector Start = CharacterData.Location + Params.TraceOriginOffset;
        FVector End = Start + CharacterData.ForwardDirection * Params.TraceForwardDistance + Params.TraceEndOffset;
        FHitResult HitResult;

        System::CapsuleTraceSingle(
			Start,
			End,
			Params.TraceRadius,
			Params.TraceHalfHeight,
			ETraceTypeQuery::TraceTypeQuery1, // Custom trace channel for traversal
            false,
            TArray<AActor>(),
            DrawDebugLevel >= 2 ? DrawDebugType : EDrawDebugTrace::None,
            HitResult,
            true,
            FLinearColor::Black,
            FLinearColor::Black,
            DrawDebugDuration);

        AFVTraversable HitActor = Cast<AFVTraversable>(HitResult.GetActor()); 

        if (HitResult.bBlockingHit == true && HitActor != nullptr)
        {
            TraverseResult.HitComponent = HitResult.GetComponent();        
            FFVCheckLedgeResult LedgeResult = HitActor.GetLedgeTransforms(HitResult.ImpactPoint, CharacterData.Location);

            if (DrawDebugLevel >= 1)
            {
                DrawDebug(LedgeResult);
            }

            if (LedgeResult.bHasFrontLedge)
            {
                TraverseResult.bHasFrontLedge = true;
                TraverseResult.FrontLedgeLocation = LedgeResult.FrontLocation;
                TraverseResult.FrontLedgeNormal = LedgeResult.FrontLedgeNormal;
                TraverseResult.bHasBackLedge = LedgeResult.bHasBackLedge;
                TraverseResult.BackLedgeLocation = LedgeResult.BackLocation;
                TraverseResult.BackLedgeNormal = LedgeResult.BackLedgeNormal;
                TraverseResult.ObstacleHeight = 
                    Math::Abs(CharacterData.Location.Z - CharacterData.CapsuleHalfHeight - LedgeResult.FrontLocation.Z);

                return true;
            }
        }

        return false;
    }

    bool DoClearanceTraces(
		FFVTraversalCharacterData CharacterData,
		FFVTraversalCheckResult& TraverseResult,
		EDrawDebugTrace DrawDebugType,
		FHitResult& OutBackLedgeClearanceHit)
    {
        float CapsuleOffset = CharacterData.CapsuleRadius + Config.CapsuleOffsetDistance;
        FVector CapsuleHalfHeightOffset = FVector(0, 0, CharacterData.CapsuleHalfHeight + Config.CapsuleOffsetDistance);
        
        // Front ledge clearance trace - checks if character can fit from current position to above front ledge
        FHitResult HitResult;
        FVector Start = CharacterData.Location;
        FVector End = TraverseResult.FrontLedgeLocation + (TraverseResult.FrontLedgeNormal * CapsuleOffset) + CapsuleHalfHeightOffset;

        DoCapsuleTrace(Start, End, CharacterData, DrawDebugType, HitResult);
        
        if (HitResult.bBlockingHit || HitResult.bStartPenetrating)
        {
            return false;
        }

        // Back ledge clearance trace - checks if there's room from above front ledge to above back ledge
        Start = End;
        End = TraverseResult.BackLedgeLocation + TraverseResult.BackLedgeNormal * CapsuleOffset + CapsuleHalfHeightOffset;

        DoCapsuleTrace(Start, End, CharacterData, DrawDebugType, OutBackLedgeClearanceHit);
        
        return true;
    }

    bool DoFloorTrace(
		FFVTraversalCharacterData CharacterData,
		FFVTraversalCheckResult& TraverseResult,
		EDrawDebugTrace DrawDebugType)
    {
        float CapsuleOffset = CharacterData.CapsuleRadius + Config.CapsuleOffsetDistance;
        FVector CapsuleHalfHeightOffset = FVector(0, 0, CharacterData.CapsuleHalfHeight + Config.CapsuleOffsetDistance);
        
        FHitResult HitResult;
        FVector FloorOffset = FVector(0, 0, TraverseResult.ObstacleHeight - CharacterData.CapsuleHalfHeight + Config.FloorTraceVerticalOffset);
        FVector Start = TraverseResult.BackLedgeLocation + TraverseResult.BackLedgeNormal * CapsuleOffset + CapsuleHalfHeightOffset;
        FVector End = TraverseResult.BackLedgeLocation + TraverseResult.BackLedgeNormal * CapsuleOffset - FloorOffset;

        DoCapsuleTrace(Start, End, CharacterData, DrawDebugType, HitResult);

        if (HitResult.bBlockingHit)
        {
            TraverseResult.bHasBackFloor = true;
            TraverseResult.BackFloorLocation = HitResult.ImpactPoint;
            TraverseResult.BackLedgeHeight = Math::Abs(HitResult.ImpactPoint.Z - TraverseResult.BackLedgeLocation.Z);
            return true;
        }

        return false;
    }

	bool DoCapsuleTrace(
		FVector Start,
		FVector End,
		FFVTraversalCharacterData CharacterData,
		EDrawDebugTrace DrawDebugType,
		FHitResult& OutHitResult)
    {
        return System::CapsuleTraceSingle(
				Start,
				End,
				CharacterData.CapsuleRadius,
				CharacterData.CapsuleHalfHeight,
				ETraceTypeQuery::Visibility, // Custom trace channel for traversal
				false,
				TArray<AActor>(),
				DrawDebugLevel >= 3 ? DrawDebugType : EDrawDebugTrace::None,
				OutHitResult,
				true,
				FLinearColor::Red,
				FLinearColor::Green,
				DrawDebugDuration);

    }

    void DrawDebug(FFVCheckLedgeResult LedgeResult)
    {
        if (LedgeResult.bHasFrontLedge)
		{
			System::DrawDebugSphere(
				LedgeResult.FrontLocation,
				10.0f,
				12,
				FLinearColor::Green,
				DrawDebugDuration);
		}

        if (LedgeResult.bHasBackLedge)
		{
			System::DrawDebugSphere(
				LedgeResult.BackLocation,
				10.0f,
				12,
				FLinearColor::Teal,
			DrawDebugDuration);
        }
	}

    FFVTraceParameters CalculateTraceParameters(EMovementMode MovementMode, FRotator Rotation, FVector Velocity)
    {
        FFVTraceParameters TraceParameters;

        TraceParameters.TraceRadius = Config.TraceRadius;

        switch (MovementMode)
        {
            case EMovementMode::MOVE_None:
            case EMovementMode::MOVE_Walking:
            case EMovementMode::MOVE_NavWalking:
            case EMovementMode::MOVE_Swimming:
            case EMovementMode::MOVE_Custom:
                TraceParameters.TraceForwardDistance = Math::GetMappedRangeValueClamped(
                    Config.GroundSpeedRange,
                    FVector2D(Config.GroundTraceForwardDistanceMin, Config.GroundTraceForwardDistanceMax), 
                    Rotation.UnrotateVector(Velocity).X);
                TraceParameters.TraceOriginOffset = Config.GroundTraceOriginOffset;
                TraceParameters.TraceEndOffset = Config.GroundTraceEndOffset;
                TraceParameters.TraceHalfHeight = Config.GroundTraceHalfHeight;
                break;
            case EMovementMode::MOVE_Falling:
            case EMovementMode::MOVE_Flying:
                TraceParameters.TraceForwardDistance = Config.AirTraceForwardDistance;
                TraceParameters.TraceOriginOffset = Config.AirTraceOriginOffset;
                TraceParameters.TraceEndOffset = Config.AirTraceEndOffset;
                TraceParameters.TraceHalfHeight = Config.AirTraceHalfHeight;
                break;
        }

        return TraceParameters;
    }

    EFVMovementMode GetTraversalMovementMode(EMovementMode MovementMode)
    {
        switch (MovementMode)
        {
            case EMovementMode::MOVE_None:
            case EMovementMode::MOVE_Walking:
            case EMovementMode::MOVE_NavWalking:
            case EMovementMode::MOVE_Swimming:
            case EMovementMode::MOVE_Custom:
                return EFVMovementMode::OnGround;
            case EMovementMode::MOVE_Falling:
            case EMovementMode::MOVE_Flying:
                return EFVMovementMode::InAir;
            default:
                return EFVMovementMode::OnGround;
        }
    }
};