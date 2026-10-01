// Configuration data structures for movement system

// Gait configuration for ground movement
struct FFVGaitConfig
{
    UPROPERTY(Category = "Speeds")
    FVector Speeds = FVector(180.f, 160.f, 150.f); // X=Forward, Y=Strafe, Z=Backward

    UPROPERTY(Category = "Acceleration")
    float Acceleration = 800.f;

    UPROPERTY(Category = "Friction")
    float Friction = 5.f;

    UPROPERTY(Category = "Acceleration")
    FVector2D SpeedRangeForAcceleration = FVector2D(300.f, 700.f);

    UPROPERTY(Category = "Acceleration")
    FVector2D AccelerationRange = FVector2D(800.f, 300.f);

    UPROPERTY(Category = "Friction")
    FVector2D SpeedRangeForFriction = FVector2D(0.f, 500.f);

    UPROPERTY(Category = "Friction")
    FVector2D FrictionRange = FVector2D(5.f, 3.f);
};

// Configuration for ground movement mode
class UFVGroundMovementConfig : UFVMovementHandlerConfigBase
{
    UPROPERTY()
    FFVGaitConfig WalkConfig;
    default WalkConfig.Speeds = FVector(180.f, 160.f, 150.f);
    default WalkConfig.Acceleration = 800.f;
    default WalkConfig.Friction = 5.f;

    UPROPERTY()
    FFVGaitConfig JogConfig;
    default JogConfig.Speeds = FVector(310.f, 250.f, 250.f);
    default JogConfig.Acceleration = 800.f;
    default JogConfig.Friction = 5.f;

    UPROPERTY()
    FFVGaitConfig SprintConfig;
    default SprintConfig.Speeds = FVector(700.f, 700.f, 700.f);
    default SprintConfig.Acceleration = 300.f;
    default SprintConfig.Friction = 3.f;
    default SprintConfig.SpeedRangeForAcceleration = FVector2D(300.f, 700.f);
    default SprintConfig.AccelerationRange = FVector2D(800.f, 300.f);
    default SprintConfig.SpeedRangeForFriction = FVector2D(0.f, 500.f);
    default SprintConfig.FrictionRange = FVector2D(5.f, 3.f);

    UPROPERTY()
    FVector CrouchSpeeds = FVector(225.f, 200.f, 180.f);

    UPROPERTY()
    float BrakingWithInput = 500.f;

    UPROPERTY()
    float BrakingWithoutInput = 2000.f;

    UPROPERTY()
    UCurveFloat StrafeSpeedMapCurve;

    UPROPERTY()
    float SprintAngleThreshold = 50.f;
};

// Configuration for traversal system
class UFVTraversalConfig : UFVMovementHandlerConfigBase
{
    UPROPERTY(Category = "Trace|Ground")
    float GroundTraceForwardDistanceMin = 75.f;

    UPROPERTY(Category = "Trace|Ground")
    float GroundTraceForwardDistanceMax = 350.f;

    UPROPERTY(Category = "Trace|Ground")
    FVector2D GroundSpeedRange = FVector2D(0.f, 500.f);

    UPROPERTY(Category = "Trace|Ground")
    FVector GroundTraceOriginOffset = FVector(0.f, 0.f, 0.f);

    UPROPERTY(Category = "Trace|Ground")
    FVector GroundTraceEndOffset = FVector(0.f, 0.f, 0.f);

    UPROPERTY(Category = "Trace|Ground")
    float GroundTraceHalfHeight = 60.f;

    UPROPERTY(Category = "Trace|Air")
    float AirTraceForwardDistance = 75.f;

    UPROPERTY(Category = "Trace|Air")
    FVector AirTraceOriginOffset = FVector(0.f, 0.f, 0.f);

    UPROPERTY(Category = "Trace|Air")
    FVector AirTraceEndOffset = FVector(0.f, 0.f, 50.f);

    UPROPERTY(Category = "Trace|Air")
    float AirTraceHalfHeight = 86.f;

    UPROPERTY(Category = "Trace|Common")
    float TraceRadius = 30.f;

    UPROPERTY(Category = "Clearance")
    float CapsuleOffsetDistance = 2.f;

    UPROPERTY(Category = "Clearance")
    float FloorTraceVerticalOffset = 50.f;

    UPROPERTY(Category = "Motion Matching")
    UChooserTable TraversalChooserTable;
};

// Configuration for traversable obstacles
class UFVTraversableConfig : UDataAsset
{
    UPROPERTY(Category = "Ledge")
    float MinLedgeWidth = 60.f;

    UPROPERTY(Category = "Ledge")
    float LedgeNormalOffset = 10.f;
};
