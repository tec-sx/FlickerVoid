#pragma once

#include "FVCharacterMovementTypes.generated.h"

UENUM(BlueprintType)
enum class EFVMovementState : uint8
{
	Idle,
	Moving
};

UENUM(BlueprintType)
enum class EFVRotationMode : uint8
{
	OrientToMovement,
	Strafe,
	Aim
};

UENUM(BlueprintType)
enum class EFVMovementHipDirection : uint8
{
	Forward,
	Backward,
	Left_HipLeft,
	Left_HipRight,
	Right_HipLeft,
	Right_HipRight
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FMoveDirectionThresholds
{
	GENERATED_BODY();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) 
	float FrontLeft = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) 
	float FrontRight = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) 
	float BackLeft = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) 
	float BackRight = 0.f;
};