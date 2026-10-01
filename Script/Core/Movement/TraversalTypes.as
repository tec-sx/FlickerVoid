enum EFVTraversalActionType
{
    Hurdle,
    Vault,
    Mantle,
}

struct FFVTraversalCharacterData
{
    UPROPERTY()
    float CapsuleHalfHeight;

    UPROPERTY()
    float CapsuleRadius;

    UPROPERTY()
    FVector ForwardDirection;

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    FVector Velocity;

    UPROPERTY()
    float Speed;

    UPROPERTY()
    EMovementMode MovementMode;

    UPROPERTY()
    UMotionWarpingComponent MotionWarping;

    UPROPERTY()
    USkeletalMeshComponent Mesh;
}

struct FFVTraversalCheckResult
{
    UPROPERTY()
    EFVTraversalActionType ActionType;

    UPROPERTY()
    bool bHasFrontLedge;

    UPROPERTY()
    FVector FrontLedgeLocation;

    UPROPERTY()
    FVector FrontLedgeNormal;

    UPROPERTY()
    bool bHasBackLedge;

    UPROPERTY()
    FVector BackLedgeLocation;

    UPROPERTY()
    FVector BackLedgeNormal;

    UPROPERTY()
    bool bHasBackFloor;

    UPROPERTY()
    FVector BackFloorLocation;

    UPROPERTY()
    float ObstacleHeight;

    UPROPERTY()
    float ObstacleDepth;

    UPROPERTY()
    float BackLedgeHeight;

    UPROPERTY()
    UPrimitiveComponent HitComponent;

    UPROPERTY()
    FFVResult ValidationResult;

    UPROPERTY()
    UAnimMontage MontageToPlay;

    UPROPERTY()
    float StartTime;

    UPROPERTY()
    float PlayRate;
}

struct FFVTraversalChooserInput
{
    UPROPERTY()
    EFVTraversalActionType ActionType;

    UPROPERTY()
    bool bHasFrontLedge;

    UPROPERTY()
    bool bHasBackLedge;

    UPROPERTY()
    bool bHasBackFloor;

     UPROPERTY()
    float ObstacleHeight;

    UPROPERTY()
    float ObstacleDepth;

    UPROPERTY()
    float BackLedgeHeight;

    UPROPERTY()
    float DistanceToLedge;

    UPROPERTY()
    EFVMovementMode MovementMode;

    UPROPERTY()
    EFVGait Gait;

    UPROPERTY()
    float Speed;

    UPROPERTY()
    FPoseHistoryReference PoseHistory;
}

struct FFVTraversalChooserOutput
{
    UPROPERTY()
    EFVTraversalActionType ActionType;

    UPROPERTY()
    float MontageStartTime;

    UPROPERTY()
    UAnimMontage MontageToPlay;
}