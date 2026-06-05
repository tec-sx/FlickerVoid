#pragma once

#include "FVMovementTypes.generated.h"

UENUM(BlueprintType)
enum class EFVGait: uint8
{
	Walk,
	Jogging,
	Sprinting
};

UENUM(BlueprintType)
enum class EFVMovementState : uint8
{
	Idle,
	Moving
};

UENUM(BlueprintType)
enum class EFVStance : uint8
{
	Stand,
	Crouch
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

UENUM(BlueprintType)
enum class EFVMovementMode : uint8
{
	None,
	OnGround,
	InAir,
	Sliding,
	Traversing,
	Swimming
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FMoveDirectionThresholds
{
	GENERATED_BODY();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float FrontLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float FrontRight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float BackLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) float BackRight;

	FMoveDirectionThresholds()
		: FrontLeft(0)
		, FrontRight(0)
		, BackLeft(0)
		, BackRight(0)
	{}
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVCharacterMovementIntent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Direction = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToJog = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToSprint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToStrafe = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToAim = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWantsToCrouch = false;
};