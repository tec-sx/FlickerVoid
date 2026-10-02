#pragma once

#include "CoreMinimal.h"
#include "FVCharacterTypes.h"
#include "FVTraversalTypes.generated.h"

class UAnimMontage;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EFVTraversalActionTypeNative : uint8
{
Hurdle,
Vault,
Mantle
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVLedgeResult
{
GENERATED_BODY()

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
bool bHasFrontLedge = false;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
FVector FrontLocation = FVector::ZeroVector;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
FVector FrontNormal = FVector::UpVector;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
bool bHasBackLedge = false;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
FVector BackLocation = FVector::ZeroVector;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
FVector BackNormal = FVector::UpVector;
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVTraversalCheckResultNative
{
GENERATED_BODY()

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
EFVTraversalActionTypeNative ActionType = EFVTraversalActionTypeNative::Mantle;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
FFVLedgeResult Ledges;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
bool bHasBackFloor = false;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
FVector BackFloorLocation = FVector::ZeroVector;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float ObstacleHeight = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float ObstacleDepth = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float BackLedgeHeight = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
TObjectPtr<UPrimitiveComponent> HitComponent;
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVTraversalChooserInputNative
{
GENERATED_BODY()

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
EFVTraversalActionTypeNative ActionType = EFVTraversalActionTypeNative::Mantle;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
bool bHasFrontLedge = false;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
bool bHasBackLedge = false;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
bool bHasBackFloor = false;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float ObstacleHeight = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float ObstacleDepth = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float BackLedgeHeight = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float DistanceToLedge = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
EFVMovementMode MovementMode = EFVMovementMode::OnGround;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
float Speed = 0.f;

UPROPERTY(BlueprintReadOnly, Category = "Traversal")
EFVGait Gait = EFVGait::Running;
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVTraversalChooserOutputNative
{
GENERATED_BODY()

UPROPERTY(BlueprintReadWrite, Category = "Traversal")
EFVTraversalActionTypeNative ActionType = EFVTraversalActionTypeNative::Mantle;

UPROPERTY(BlueprintReadWrite, Category = "Traversal")
float MontageStartTime = 0.f;

UPROPERTY(BlueprintReadWrite, Category = "Traversal")
TObjectPtr<UAnimMontage> MontageToPlay;
};