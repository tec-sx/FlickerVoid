// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVLocomotionStyleData.generated.h"

/**
 * UFVLocomotionStyleData
 * 
 * Data asset containing locomotion style parameters for procedural animation.
 * Create derived assets in the editor for different styles (Masculine, Feminine, Tired, etc.)
 * 
 * USAGE:
 *   1. Right-click in Content Browser ? Miscellaneous ? Data Asset
 *   2. Select FVLocomotionStyleData as the class
 *   3. Name it (e.g., DA_LocomotionStyle_Feminine)
 *   4. Adjust parameters for your style
 *   5. Reference in your character/animation blueprint
 * 
 * BLENDING:
 *   Use the static Lerp() function to blend between styles at runtime.
 *   Example: Blend between Masculine and Tired based on stamina.
 */
UCLASS(BlueprintType, Blueprintable)
class FLICKERVOIDCHARACTER_API UFVLocomotionStyleData : public UDataAsset
{
    GENERATED_BODY()

public:
    UFVLocomotionStyleData();

    // ========================================================================
    // HIP PARAMETERS
    // ========================================================================

    /** 
     * Hip swing amplitude multiplier.
     * 0 = No hip swing (robotic)
     * 1 = Normal hip movement
     * 1.5+ = Exaggerated (feminine, theatrical)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip", meta = (ClampMin = "0.0", ClampMax = "2.5"))
    float HipSwingAmount = 1.0f;

    /**
     * Lateral hip sway (side-to-side).
     * Higher values create more pronounced hip sway.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float HipLateralSwing = 1.0f;

    /**
     * Vertical hip bob (up-down bounce).
     * Higher values create more bounce in the walk.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float HipVerticalBob = 1.0f;

    /**
     * Hip rotation amount (twist around vertical axis).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float HipRotation = 1.0f;

    // ========================================================================
    // SPINE PARAMETERS
    // ========================================================================

    /**
     * Overall spine bend/flexibility amount.
     * 0 = Rigid spine
     * 1 = Normal flexibility
     * Higher = More bend/sway
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float SpineBendAmount = 1.0f;

    /**
     * Spine counter-rotation against hip (stabilization).
     * Higher values keep upper body more stable while hips move.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpineCompensation = 0.5f;

    /**
     * Forward/backward spine lean.
     * Negative = lean back, Positive = lean forward
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float SpineLean = 0.0f;

    /**
     * Optional curve asset for custom spine shape.
     * If null, uses default S-curve.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine")
    TObjectPtr<UCurveFloat> SpineShapeCurve = nullptr;

    // ========================================================================
    // ARM PARAMETERS
    // ========================================================================

    /**
     * Arm swing amplitude multiplier.
     * 0 = No arm swing (robotic, hands at sides)
     * 1 = Normal swing
     * Higher = Exaggerated swing
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "3.0"))
    float ArmSwingAmount = 1.0f;

    /**
     * Arm stiffness (muscle tension).
     * 0 = Relaxed/sloppy (arms flop, wrists lag)
     * 0.5 = Natural
     * 1 = Tense/rigid (military march)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ArmStiffness = 0.5f;

    /**
     * Arm swing asymmetry.
     * -1 = Left arm swings more
     * 0 = Symmetric
     * +1 = Right arm swings more
     * Useful for carrying objects, injuries.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float ArmAsymmetry = 0.0f;

    // ========================================================================
    // LEG PARAMETERS
    // ========================================================================

    /**
     * Step width modifier.
     * -1 = Catwalk (feet cross centerline, narrow)
     * 0 = Normal shoulder-width stance
     * +1 = Wide stance (power walk, ork)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legs", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float StepWidth = 0.0f;

    /**
     * Minimum foot separation (cm) to prevent overlap.
     * Set to 0 to allow feet to cross completely.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legs", meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float MinStepSeparation = 10.0f;

    /**
     * Leg stiffness (similar to arm stiffness).
     * Affects how rigidly the feet hit their marks.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LegStiffness = 0.5f;

    /**
     * Knee bend amount during walk.
     * Higher values create more knee flex.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legs", meta = (ClampMin = "0.5", ClampMax = "1.5"))
    float KneeBend = 1.0f;

    // ========================================================================
    // GLOBAL PARAMETERS
    // ========================================================================

    /**
     * Random variation for all movements.
     * 0 = Mechanical/perfect repetition
     * 0.1 = Subtle natural variation (recommended)
     * 0.3+ = Noticeable variation (drunk, exhausted)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global", meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float Randomness = 0.1f;

    /**
     * Overall energy/intensity multiplier.
     * Scales multiple parameters together.
     * 0.5 = Low energy (tired, sneaking)
     * 1.0 = Normal
     * 1.5 = High energy (excited, hurrying)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global", meta = (ClampMin = "0.3", ClampMax = "2.0"))
    float EnergyLevel = 1.0f;

    // ========================================================================
    // FUNCTIONS
    // ========================================================================

    /**
     * Blend between two locomotion styles.
     * 
     * @param A - First style
     * @param B - Second style
     * @param Alpha - Blend factor (0 = A, 1 = B)
     * @return Blended style values
     */
    UFUNCTION(BlueprintCallable, Category = "Locomotion Style")
    static UFVLocomotionStyleData* CreateBlended(
        const UFVLocomotionStyleData* A,
        const UFVLocomotionStyleData* B,
        float Alpha);

    /**
     * Get all parameters as a struct for passing to Control Rig.
     * Useful for setting multiple rig variables at once.
     */
    UFUNCTION(BlueprintCallable, Category = "Locomotion Style")
    void GetParametersForRig(
        float& OutHipSwing,
        float& OutHipLateral,
        float& OutHipVertical,
        float& OutSpineBend,
        float& OutSpineCompensation,
        float& OutArmSwing,
        float& OutArmStiffness,
        float& OutArmAsymmetry,
        float& OutStepWidth,
        float& OutMinSeparation,
        float& OutLegStiffness,
        float& OutRandomness) const;

    /**
     * Apply energy level scaling to get effective values.
     * Some parameters scale with energy, others don't.
     */
    UFUNCTION(BlueprintCallable, Category = "Locomotion Style")
    float GetEffectiveArmSwing() const { return ArmSwingAmount * EnergyLevel; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion Style")
    float GetEffectiveHipSwing() const { return HipSwingAmount * FMath::Lerp(0.8f, 1.2f, EnergyLevel - 0.5f); }
};
