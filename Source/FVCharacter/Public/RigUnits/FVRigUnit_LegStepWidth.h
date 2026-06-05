// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "FVRigUnit_LegStepWidth.generated.h"

/**
 * FFVRigUnit_LegStepWidth
 * 
 * Phase-driven leg step width modifier with overlap prevention.
 * Controls lateral foot placement from catwalk (narrow) to wide stance.
 * 
 * STEP WIDTH:
 *   -1.0 = Catwalk - feet cross the centerline (feminine, runway model)
 *    0.0 = Normal - natural shoulder-width stance
 *   +1.0 = Wide - exaggerated wide stance (masculine, ork, power stance)
 * 
 * OVERLAP PREVENTION:
 *   When feet would cross too close, automatically pushes them apart.
 *   Uses MinSeparation parameter to define minimum allowed distance.
 * 
 * PHASE COORDINATION:
 *   - Uses plant weight curves to know which foot is grounded
 *   - Planted foot: minimal movement (stable base)
 *   - Swinging foot: applies lateral offset
 * 
 * INPUTS:
 *   Phase (-1 to 1) - Walk cycle phase
 *   LeftPlantWeight (0-1) - From left_leg_plant curve (1 = fully planted)
 *   RightPlantWeight (0-1) - From right_leg_plant curve (1 = fully planted)
 */
USTRUCT(meta = (DisplayName = "FV Leg Step Width", Category = "FlickerVoid|Locomotion",
    Keywords = "Leg,Foot,Step,Width,Catwalk,Wide,Stance,Overlap"))
struct FLICKERVOIDCHARACTER_API FFVRigUnit_LegStepWidth : public FRigUnitMutable
{
    GENERATED_BODY()

    FFVRigUnit_LegStepWidth()
        : Phase(0.0f)
        , LeftPlantWeight(1.0f)
        , RightPlantWeight(1.0f)
        , StepWidth(0.0f)
        , MinSeparation(10.0f)
        , Stiffness(0.5f)
        , Randomness(0.05f)
        , bDrawDebug(false)
        , BlendWeight(1.0f)
    {}

    RIGVM_METHOD()
    virtual void Execute() override;

    virtual FRigVMStructUpgradeInfo GetUpgradeInfo() const override;

    // ========================================================================
    // REFERENCE INPUTS
    // ========================================================================

    /** Left foot IK control (will be modified) */
    UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
    FRigElementKey LeftFootIKControl;

    /** Right foot IK control (will be modified) */
    UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
    FRigElementKey RightFootIKControl;

    /** Pelvis/root reference for determining lateral direction */
    UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
    FRigElementKey PelvisReference;

    // ========================================================================
    // PHASE & PLANT INPUTS (from animation curves)
    // ========================================================================

    /** Walk cycle phase (-1 to 1) */
    UPROPERTY(meta = (Input, ClampMin = "-1.0", ClampMax = "1.0"))
    float Phase;

    /** 
     * Left foot plant weight (0-1) from left_leg_plant curve.
     * 1.0 = fully planted/grounded, 0.0 = fully in swing
     */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float LeftPlantWeight;

    /** 
     * Right foot plant weight (0-1) from right_leg_plant curve.
     * 1.0 = fully planted/grounded, 0.0 = fully in swing
     */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float RightPlantWeight;

    // ========================================================================
    // STEP WIDTH PARAMETERS
    // ========================================================================

    /**
     * Step width modifier.
     * -1.0 = Catwalk (feet cross centerline)
     *  0.0 = Normal stance
     * +1.0 = Wide stance
     */
    UPROPERTY(meta = (Input, ClampMin = "-1.0", ClampMax = "1.0"))
    float StepWidth;

    /**
     * Minimum horizontal separation between feet (cm).
     * Prevents feet from overlapping regardless of StepWidth.
     * Set to 0 to disable overlap prevention.
     */
    UPROPERTY(meta = (Input, ClampMin = "0.0"))
    float MinSeparation;

    /**
     * Movement stiffness (0 = sloppy, 1 = rigid).
     * Affects how quickly feet reach their target lateral position.
     */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float Stiffness;

    /**
     * Random variation for natural feel.
     */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "0.3"))
    float Randomness;

    /** Draw debug visualization */
    UPROPERTY(meta = (Input))
    bool bDrawDebug;

    /** Overall blend weight */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float BlendWeight;

    // ========================================================================
    // OUTPUTS
    // ========================================================================

    /** Modified left foot IK target position */
    UPROPERTY(meta = (Output))
    FVector LeftFootTargetPosition;

    /** Modified right foot IK target position */
    UPROPERTY(meta = (Output))
    FVector RightFootTargetPosition;

    /** Actual separation distance between feet (for debugging) */
    UPROPERTY(meta = (Output))
    float CurrentSeparation;

    /** Was overlap correction applied this frame */
    UPROPERTY(meta = (Output))
    bool bOverlapCorrected;

    // ========================================================================
    // SIMULATION STATE
    // ========================================================================

    UPROPERTY(transient)
    FVector LeftFootVelocity = FVector::ZeroVector;

    UPROPERTY(transient)
    FVector LeftFootOffset = FVector::ZeroVector;

    UPROPERTY(transient)
    FVector RightFootVelocity = FVector::ZeroVector;

    UPROPERTY(transient)
    FVector RightFootOffset = FVector::ZeroVector;

    UPROPERTY(transient)
    float AccumulatedTime = 0.0f;

    UPROPERTY(transient)
    bool bInitialized = false;
};

/**
 * FFVRigUnit_LegStepWidthSimple
 * 
 * Simplified version that directly modifies foot controls without spring physics.
 * Use when you want immediate response without secondary motion.
 */
USTRUCT(meta = (DisplayName = "FV Leg Step Width (Simple)", Category = "FlickerVoid|Locomotion",
    Keywords = "Leg,Foot,Step,Width,Simple"))
struct FLICKERVOIDCHARACTER_API FFVRigUnit_LegStepWidthSimple : public FRigUnitMutable
{
    GENERATED_BODY()

    RIGVM_METHOD()
    virtual void Execute() override;

    virtual FRigVMStructUpgradeInfo GetUpgradeInfo() const override;

    /** Left foot IK control */
    UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
    FRigElementKey LeftFootIKControl;

    /** Right foot IK control */
    UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
    FRigElementKey RightFootIKControl;

    /** Pelvis reference for lateral direction */
    UPROPERTY(meta = (Input, CustomWidget = "BoneName"))
    FRigElementKey PelvisReference;

    /** Left foot plant weight (0-1) */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float LeftPlantWeight = 1.0f;

    /** Right foot plant weight (0-1) */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float RightPlantWeight = 1.0f;

    /** Step width (-1 catwalk, 0 normal, +1 wide) */
    UPROPERTY(meta = (Input, ClampMin = "-1.0", ClampMax = "1.0"))
    float StepWidth = 0.0f;

    /** Minimum separation (cm) */
    UPROPERTY(meta = (Input, ClampMin = "0.0"))
    float MinSeparation = 10.0f;

    /** Blend weight */
    UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0"))
    float BlendWeight = 1.0f;

    /** Modified left foot position */
    UPROPERTY(meta = (Output))
    FVector LeftFootTargetPosition;

    /** Modified right foot position */
    UPROPERTY(meta = (Output))
    FVector RightFootTargetPosition;
};
