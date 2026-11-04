#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "DefaultMovementSet/Modes/WalkingMode.h"
#include "FVGroundMovementMode.generated.h"

struct FFVLocomotionInputs;

/** Speeds and acceleration of one gait. */
USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Speed moving forward, in cm/s."))
	float ForwardSpeed = 310.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Speed moving sideways, in cm/s, while facing the camera."))
	float StrafeSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Speed moving backwards, in cm/s, while facing the camera."))
	float BackwardSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Acceleration towards the desired speed, in cm/s²."))
	float Acceleration = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Deceleration without move input, in cm/s²."))
	float Deceleration = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ToolTip = "Ground friction in this gait. Lower values make turns wider."))
	float GroundFriction = 5.f;
};

/**
 * Mover walking mode with FlickerVoid gaits: picks the walk, run, sprint or crouch speeds from the locomotion inputs and
 * writes them into the Common Legacy Movement Settings before Mover's walking logic runs. Subclass it in AngelScript
 * for ground movement with other rules.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "FV Ground Movement Mode"))
class FLICKERVOIDCHARACTER_API UFVGroundMovementMode : public UWalkingMode
{
	GENERATED_BODY()

public:
	UFVGroundMovementMode(const FObjectInitializer& ObjectInitializer);

	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

	UFUNCTION(BlueprintPure, Category = "Gait", meta = (ToolTip = "The gait settings these inputs select, crouch first."))
	FFVGaitSettings SelectGait(const FFVLocomotionInputs& Inputs, bool bCrouching) const;

	UFUNCTION(BlueprintPure, Category = "Gait", meta = (ToolTip = "Speed for the move direction: forward speed, or blended towards strafe and backward speed while facing the camera."))
	static float GetDirectionalSpeed(const FFVGaitSettings& Gait, const FFVLocomotionInputs& Inputs);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Walk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Run;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait")
	FFVGaitSettings Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0", ToolTip = "How fast the character turns towards its orientation intent, in degrees per second."))
	float TurningRate = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ToolTip = "Optional attribute on the owner's ability system that multiplies every gait speed (1 = unchanged)."))
	FGameplayAttribute SpeedMultiplierAttribute;

private:
	float GetSpeedMultiplier(const AActor* Owner) const;
};
