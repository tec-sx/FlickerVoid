#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Engine/DataAsset.h"
#include "FVLocomotionData.generated.h"

/** Speeds and acceleration of one gait. */
USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Speed moving forward, in cm/s."))
	float ForwardSpeed = 310.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Speed moving sideways, in cm/s. Used while facing the camera (aiming or strafing)."))
	float StrafeSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Speed moving backwards, in cm/s. Used while facing the camera (aiming or strafing)."))
	float BackwardSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Acceleration towards the desired speed, in cm/s²."))
	float Acceleration = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Deceleration when there is no move input, in cm/s²."))
	float Deceleration = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Ground friction while moving in this gait. Lower values make turns wider."))
	float GroundFriction = 5.f;
};

/**
 * Locomotion tuning for one kind of character: gait speeds, crouch speed, turning and the attribute that scales speed.
 * Mover reads the active gait through its Common Legacy Movement Settings, which the locomotion component writes.
 */
UCLASS(BlueprintType, Const)
class FLICKERVOIDCHARACTER_API UFVLocomotionData : public UDataAsset
{
	GENERATED_BODY()

public:
	UFVLocomotionData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Walk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Run;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ClampMax = "180", ToolTip = "Sprint only while the move input is within this angle of the facing direction, in degrees."))
	float SprintAngleThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0", ToolTip = "How fast the character turns towards its orientation intent, in degrees per second."))
	float TurningRate = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ToolTip = "Optional attribute on the owner's ability system that multiplies every gait speed (1 = unchanged)."))
	FGameplayAttribute SpeedMultiplierAttribute;
};
