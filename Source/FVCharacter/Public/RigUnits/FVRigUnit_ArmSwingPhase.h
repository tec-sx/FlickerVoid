// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "FVRigUnit_ArmSwingPhase.generated.h"

/**
 * FFVRigUnit_ArmSwingPhase
 * 
 * Phase-driven arm swing modifier with spring-damper physics simulation.
 * Creates natural arm motion from relaxed/sloppy to tense/stiff.
 * 
 * PHASE CONVENTION (-1 to 1):
 *   -1.0 = Right foot contact (right arm back)
 *    0.0 = Passing position
 *   +1.0 = Left foot contact (right arm forward)
 * 
 * SWING AMOUNT:
 *   0.0 = No swing - robotic, arms frozen in place
 *   1.0 = Normal swing - matches original animation amplitude
 *   2.0 = Exaggerated - feminine, expressive, theatrical
 * 
 * STIFFNESS (Muscle Tension):
 *   0.0 = Very relaxed - arms flop loosely, wrists lag and overshoot
 *         Elbows bend freely, hands "flap" at swing reversals
 *   0.5 = Natural walk - balanced follow-through
 *   1.0 = Very tense - military march, arms rigid, minimal secondary motion
 * 
 * RANDOMNESS:
 *   Adds subtle variation to prevent mechanical repetition.
 *   0.0 = Perfect machine-like consistency
 *   0.1 = Subtle natural variation (recommended)
 *   0.3 = Noticeable variation (drunk, exhausted, injured)
 */
USTRUCT(meta = (DisplayName = "FV Arm Swing (Phase)", Category = "FlickerVoid|Locomotion", 
	Keywords = "Arm,Swing,Walk,Run,Phase,IK,Stiffness,Relaxed"))
struct FLICKERVOIDCHARACTER_API FFVRigUnit_ArmSwingPhase : public FRigUnitMutable
{
	GENERATED_BODY()

	FFVRigUnit_ArmSwingPhase()
		: Phase(0.0f)
		, SwingAmount(1.0f)
		, Stiffness(0.5f)
		, Randomness(0.1f)
		, RandomSeed(0)
		, bIsLeftArm(false)
		, bDrawDebug(false)
		, BlendWeight(1.0f)
	{}

	RIGVM_METHOD()
	virtual void Execute() override;

	virtual FRigVMStructUpgradeInfo GetUpgradeInfo() const override;

	// ========================================================================
	// REFERENCE INPUTS - Animation buffer controls
	// ========================================================================

	/** Wrist reference control - the animated pose before modification */
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey WristReference;

	/** Elbow pole reference control */
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey ElbowPoleReference;

	/** Shoulder reference for coordinate frame */
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey ShoulderReference;

	// ========================================================================
	// CORE PARAMETERS
	// ========================================================================

	/**
	 * Walk cycle phase (-1 to 1).
	 * Sample your phase curve and connect here.
	 */
	UPROPERTY(meta = (Input, ClampMin = "-1.0", ClampMax = "1.0"))
	float Phase;

	/**
	 * Swing amplitude multiplier.
	 * 0 = no swing (robot), 1 = normal, >1 = exaggerated
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "3.0"))
	float SwingAmount;

	/**
	 * Arm stiffness / muscle tension.
	 * 0 = sloppy/relaxed, 0.5 = natural, 1 = tense/rigid
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
	float Stiffness;

	/**
	 * Random variation amount for natural feel.
	 * 0 = mechanical, 0.1 = natural, 0.3 = erratic
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "0.5"))
	float Randomness;

	/** Seed for deterministic randomness (use limb index or character ID) */
	UPROPERTY(meta = (Input))
	int32 RandomSeed;

	/** Is this the left arm? Auto-inverts phase for coordination. */
	UPROPERTY(meta = (Input))
	bool bIsLeftArm;

	/** Draw debug visualization */
	UPROPERTY(meta = (Input))
	bool bDrawDebug;

	/** Overall blend weight */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
	float BlendWeight;

	// ========================================================================
	// OUTPUTS
	// ========================================================================

	/** Wrist IK target position (world space) */
	UPROPERTY(meta = (Output))
	FVector WristTargetPosition;

	/** Wrist IK target rotation */
	UPROPERTY(meta = (Output))
	FQuat WristTargetRotation;

	/** Elbow pole vector target */
	UPROPERTY(meta = (Output))
	FVector ElbowPoleTarget;

	/** Swing factor (-1 to 1) for driving other effects */
	UPROPERTY(meta = (Output))
	float SwingFactor;

	// ========================================================================
	// SIMULATION STATE (Spring-Damper)
	// ========================================================================

	UPROPERTY(transient)
	FVector WristVelocity = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector WristSimOffset = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector ElbowVelocity = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector ElbowSimOffset = FVector::ZeroVector;

	UPROPERTY(transient)
	FRotator WristRotVelocity = FRotator::ZeroRotator;

	UPROPERTY(transient)
	FRotator WristSimRotation = FRotator::ZeroRotator;

	UPROPERTY(transient)
	float AccumulatedTime = 0.0f;

	UPROPERTY(transient)
	bool bInitialized = false;
};

/**
 * FFVRigUnit_ArmSwingPhasePair
 * 
 * Processes BOTH arms with automatic phase coordination.
 */
USTRUCT(meta = (DisplayName = "FV Arm Swing Pair (Phase)", Category = "FlickerVoid|Locomotion",
	Keywords = "Arm,Swing,Walk,Run,Phase,Pair,Both"))
struct FLICKERVOIDCHARACTER_API FFVRigUnit_ArmSwingPhasePair : public FRigUnitMutable
{
	GENERATED_BODY()

	FFVRigUnit_ArmSwingPhasePair()
		: Phase(0.0f)
		, SwingAmount(1.0f)
		, Stiffness(0.5f)
		, Randomness(0.1f)
		, Asymmetry(0.0f)
		, bDrawDebug(false)
		, BlendWeight(1.0f)
	{}

	RIGVM_METHOD()
	virtual void Execute() override;

	virtual FRigVMStructUpgradeInfo GetUpgradeInfo() const override;

	// ========== Left Arm References ==========
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey LeftWristReference;

	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey LeftElbowPoleReference;

	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey LeftShoulderReference;

	// ========== Right Arm References ==========
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey RightWristReference;

	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey RightElbowPoleReference;

	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FRigElementKey RightShoulderReference;

	// ========== Parameters ==========
	
	/** Walk cycle phase (-1 to 1) */
	UPROPERTY(meta = (Input, ClampMin = "-1.0", ClampMax = "1.0"))
	float Phase;

	/** Swing amplitude (0=none, 1=normal, >1=exaggerated) */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "3.0"))
	float SwingAmount;

	/** Arm stiffness (0=sloppy, 0.5=natural, 1=tense) */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
	float Stiffness;

	/** Random variation amount */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "0.5"))
	float Randomness;

	/** Asymmetry: -1=left more, 0=equal, 1=right more */
	UPROPERTY(meta = (Input, ClampMin = "-1.0", ClampMax = "1.0"))
	float Asymmetry;

	UPROPERTY(meta = (Input))
	bool bDrawDebug;

	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
	float BlendWeight;

	// ========== Outputs ==========
	UPROPERTY(meta = (Output))
	FVector LeftWristTarget;

	UPROPERTY(meta = (Output))
	FQuat LeftWristRotation;

	UPROPERTY(meta = (Output))
	FVector LeftElbowPoleTarget;

	UPROPERTY(meta = (Output))
	FVector RightWristTarget;

	UPROPERTY(meta = (Output))
	FQuat RightWristRotation;

	UPROPERTY(meta = (Output))
	FVector RightElbowPoleTarget;

	UPROPERTY(meta = (Output))
	float LeftSwingFactor;

	UPROPERTY(meta = (Output))
	float RightSwingFactor;

	// ========== Simulation State ==========
	UPROPERTY(transient)
	FVector LeftWristVelocity = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector LeftWristSimOffset = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector LeftElbowVelocity = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector LeftElbowSimOffset = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector RightWristVelocity = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector RightWristSimOffset = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector RightElbowVelocity = FVector::ZeroVector;

	UPROPERTY(transient)
	FVector RightElbowSimOffset = FVector::ZeroVector;

	UPROPERTY(transient)
	float AccumulatedTime = 0.0f;

	UPROPERTY(transient)
	bool bInitialized = false;
};
