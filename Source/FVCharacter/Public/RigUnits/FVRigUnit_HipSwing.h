// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "FVRigUnit_HipSwing.generated.h"

/**
 * FFVRigUnit_HipSwing
 * 
 * Modifies hip swing amplitude in walk cycle animations.
 * Outputs foot effector targets for use with TwoBoneIK nodes.
 * 
 * USAGE IN CONTROL RIG GRAPH:
 *   1. Connect this node's foot outputs to TwoBoneIK effector inputs
 *   2. TwoBoneIK handles leg solving with proper bone axis support
 * 
 * SwingAmount values:
 *   1.0 = Original animation
 *   0.0 = No hip swing (robotic)
 *   1.5 = Exaggerated swing (feminine)
 *   0.5 = Reduced swing (masculine/military)
 */
USTRUCT(meta = (DisplayName = "FV Hip Swing", Category = "FlickerVoid|Animation", Keywords = "Hip,Pelvis,Swing,Walk,Feminine,Masculine"))
struct FLICKERVOIDCHARACTER_API FFVRigUnit_HipSwing : public FRigUnitMutable
{
	GENERATED_BODY()

	FFVRigUnit_HipSwing()
		: SwingAmount(1.0f)
		, VerticalBobAmount(1.0f)
		, SpineFlexibility(0.5f)
		, BlendWeight(1.0f)
		, bInitialized(false)
	{}

	RIGVM_METHOD()
	virtual void Execute() override;

	// ========================================================================
	// BONE INPUTS
	// ========================================================================

	/** The pelvis/hip bone to modify */
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FName PelvisBone;

	/** Left foot bone - used to capture target position before pelvis modification */
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FName LeftFootBone;

	/** Right foot bone - used to capture target position before pelvis modification */
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FName RightFootBone;

	// --- Spine Chain (bottom to top, for counter-rotation) ---
	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FName Spine01Bone;

	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FName Spine02Bone;

	UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
	FName Spine03Bone;

	// ========================================================================
	// PARAMETERS
	// ========================================================================

	/**
	 * Master multiplier for hip swing rotation.
	 * 1.0 = original, <1 = reduced, >1 = exaggerated
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "2.0"))
	float SwingAmount;

	/**
	 * Multiplier for vertical hip bob (affects Z position delta).
	 * Use this to control how much the character crouches/rises.
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "2.0"))
	float VerticalBobAmount;

	/**
	 * How much the spine bends to compensate for pelvis rotation.
	 * 0.0 = rigid spine, 1.0 = full counter-bend distributed across spine
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
	float SpineFlexibility;

	/**
	 * Overall blend weight for this modifier.
	 */
	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
	float BlendWeight;

	// ========================================================================
	// OUTPUTS - Connect these to TwoBoneIK nodes in the Control Rig graph
	// ========================================================================

	/** 
	 * Left foot effector target (world space).
	 * Connect to TwoBoneIK Effector input for left leg.
	 */
	UPROPERTY(meta = (Output))
	FTransform LeftFootEffector;

	/** 
	 * Right foot effector target (world space).
	 * Connect to TwoBoneIK Effector input for right leg.
	 */
	UPROPERTY(meta = (Output))
	FTransform RightFootEffector;

	// ========================================================================
	// CACHED DATA (transient)
	// ========================================================================

	UPROPERTY(transient)
	FCachedRigElement CachedPelvis;

	UPROPERTY(transient)
	FCachedRigElement CachedLeftFoot;

	UPROPERTY(transient)
	FCachedRigElement CachedRightFoot;

	UPROPERTY(transient)
	FCachedRigElement CachedSpine01;

	UPROPERTY(transient)
	FCachedRigElement CachedSpine02;

	UPROPERTY(transient)
	FCachedRigElement CachedSpine03;

	UPROPERTY(transient)
	FQuat InitialPelvisRotation;

	UPROPERTY(transient)
	FVector InitialPelvisPosition;

	UPROPERTY(transient)
	bool bInitialized;
};
