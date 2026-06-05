// Copyright FlickerVoid. All Rights Reserved.

#include "RigUnits/FVRigUnit_LegStepWidth.h"
#include "ControlRig.h"
#include "Units/RigUnitContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRigUnit_LegStepWidth)

// ============================================================================
// Constants
// ============================================================================

namespace FVLegStepWidthConstants
{
    // Maximum lateral offset in cm (at StepWidth = ±1)
    constexpr float MaxLateralOffset = 15.0f;
    
    // Spring physics parameters
    constexpr float MinSpringK = 10.0f;
    constexpr float MaxSpringK = 60.0f;
    constexpr float MinDamping = 0.4f;
    constexpr float MaxDamping = 1.1f;
}

// ============================================================================
// Helper Functions
// ============================================================================

namespace FVLegStepWidthUtils
{
    /**
     * Generate deterministic noise.
     */
    float GenerateNoise(float Time, int32 Seed, float Frequency = 1.0f)
    {
        float SeedOffset = static_cast<float>(Seed) * 0.7123f;
        float Noise = FMath::Sin((Time * Frequency + SeedOffset) * 2.13f) * 0.5f
                    + FMath::Sin((Time * Frequency * 1.7f + SeedOffset) * 3.51f) * 0.3f
                    + FMath::Sin((Time * Frequency * 2.3f + SeedOffset) * 5.17f) * 0.2f;
        return FMath::Clamp(Noise, -1.0f, 1.0f);
    }

    /**
     * Get spring parameters from stiffness.
     */
    void GetSpringParams(float Stiffness, float& OutSpringK, float& OutDamping)
    {
        using namespace FVLegStepWidthConstants;
        float T = Stiffness * Stiffness;
        OutSpringK = FMath::Lerp(MinSpringK, MaxSpringK, T);
        OutDamping = FMath::Lerp(MinDamping, MaxDamping, Stiffness);
    }

    /**
     * Spring-damper simulation.
     */
    void SimulateSpring(
        FVector& CurrentOffset,
        FVector& Velocity,
        const FVector& TargetOffset,
        float SpringK,
        float Damping,
        float DeltaTime)
    {
        FVector Displacement = CurrentOffset - TargetOffset;
        FVector SpringForce = -SpringK * Displacement;
        FVector DampingForce = -Damping * Velocity * SpringK;
        FVector Acceleration = SpringForce + DampingForce;
        
        Velocity += Acceleration * DeltaTime;
        CurrentOffset += Velocity * DeltaTime;
        
        const float MaxVelocity = 300.0f;
        Velocity = Velocity.GetClampedToMaxSize(MaxVelocity);
    }

    /**
     * Calculate lateral offset for a foot.
     * 
     * @param StepWidth - Width modifier (-1 to 1)
     * @param PlantWeight - How planted the foot is (0 = swinging, 1 = planted)
     * @param bIsLeftFoot - Left or right foot
     * @param RightAxis - Character's right direction
     * @return Lateral offset vector
     */
    FVector CalculateLateralOffset(
        float StepWidth,
        float PlantWeight,
        bool bIsLeftFoot,
        const FVector& RightAxis)
    {
        using namespace FVLegStepWidthConstants;
        
        // Base lateral direction (left foot goes left, right foot goes right)
        float LateralDirection = bIsLeftFoot ? -1.0f : 1.0f;
        
        // Calculate offset amount
        // StepWidth > 0: Push feet outward (wide stance)
        // StepWidth < 0: Pull feet inward (narrow/catwalk)
        float OffsetAmount = StepWidth * MaxLateralOffset * LateralDirection;
        
        // Planted feet move less (stability)
        // Swinging feet get full offset applied
        float PlantFactor = FMath::Lerp(1.0f, 0.3f, PlantWeight);
        OffsetAmount *= PlantFactor;
        
        return RightAxis * OffsetAmount;
    }

    /**
     * Apply overlap prevention.
     * 
     * @param LeftPos - Left foot position (modified in place)
     * @param RightPos - Right foot position (modified in place)
     * @param RightAxis - Character's right direction
     * @param MinSeparation - Minimum allowed distance
     * @return True if correction was applied
     */
    bool PreventOverlap(
        FVector& LeftPos,
        FVector& RightPos,
        const FVector& RightAxis,
        float MinSeparation)
    {
        if (MinSeparation <= 0.0f)
        {
            return false;
        }

        // Calculate horizontal separation (project onto right axis)
        FVector Separation = RightPos - LeftPos;
        float LateralSep = FVector::DotProduct(Separation, RightAxis);
        
        // Check if feet are too close or crossing
        // LateralSep should be positive (right foot to the right of left foot)
        if (LateralSep < MinSeparation)
        {
            // Calculate how much to push apart
            float Correction = (MinSeparation - LateralSep) * 0.5f;
            
            // Push feet apart equally
            LeftPos -= RightAxis * Correction;
            RightPos += RightAxis * Correction;
            
            return true;
        }
        
        return false;
    }
}

// ============================================================================
// FFVRigUnit_LegStepWidth
// ============================================================================

FRigVMStructUpgradeInfo FFVRigUnit_LegStepWidth::GetUpgradeInfo() const
{
    return FRigVMStructUpgradeInfo();
}

FFVRigUnit_LegStepWidth_Execute()
{
    using namespace FVLegStepWidthUtils;
    
    if (BlendWeight <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
    if (Hierarchy == nullptr)
    {
        return;
    }

    float DeltaTime = ExecuteContext.GetDeltaTime();

    // Validate inputs
    if (!LeftFootIKControl.IsValid() || !RightFootIKControl.IsValid() || !PelvisReference.IsValid())
    {
        return;
    }

    // Get indices
    const int32 LeftFootIdx = Hierarchy->GetIndex(LeftFootIKControl);
    const int32 RightFootIdx = Hierarchy->GetIndex(RightFootIKControl);
    const int32 PelvisIdx = Hierarchy->GetIndex(PelvisReference);

    if (LeftFootIdx == INDEX_NONE || RightFootIdx == INDEX_NONE || PelvisIdx == INDEX_NONE)
    {
        return;
    }

    // Get current transforms
    FTransform LeftFootTransform = Hierarchy->GetGlobalTransform(LeftFootIdx);
    FTransform RightFootTransform = Hierarchy->GetGlobalTransform(RightFootIdx);
    FTransform PelvisTransform = Hierarchy->GetGlobalTransform(PelvisIdx);

    // Get character's right direction from pelvis
    FVector RightAxis = PelvisTransform.GetRotation().GetRightVector();
    RightAxis.Z = 0.0f; // Keep it horizontal
    RightAxis.Normalize();

    // Initialize
    if (!bInitialized)
    {
        LeftFootVelocity = FVector::ZeroVector;
        LeftFootOffset = FVector::ZeroVector;
        RightFootVelocity = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        AccumulatedTime = 0.0f;
        bInitialized = true;
    }

    AccumulatedTime += DeltaTime;

    // Apply randomness
    float NoiseLeft = GenerateNoise(AccumulatedTime, 100, 0.7f);
    float NoiseRight = GenerateNoise(AccumulatedTime, 200, 0.7f);
    float EffectiveStepWidth = StepWidth * (1.0f + NoiseLeft * Randomness * 0.5f);

    // Calculate target lateral offsets
    FVector LeftTargetOffset = CalculateLateralOffset(EffectiveStepWidth, LeftPlantWeight, true, RightAxis);
    FVector RightTargetOffset = CalculateLateralOffset(EffectiveStepWidth, RightPlantWeight, false, RightAxis);

    // Apply spring physics
    if (DeltaTime > 0.0f)
    {
        float SpringK, Damping;
        GetSpringParams(Stiffness, SpringK, Damping);
        
        SimulateSpring(LeftFootOffset, LeftFootVelocity, LeftTargetOffset, SpringK, Damping, DeltaTime);
        SimulateSpring(RightFootOffset, RightFootVelocity, RightTargetOffset, SpringK, Damping, DeltaTime);
    }

    // Apply offsets
    FVector LeftFootPos = LeftFootTransform.GetLocation() + LeftFootOffset * BlendWeight;
    FVector RightFootPos = RightFootTransform.GetLocation() + RightFootOffset * BlendWeight;

    // Overlap prevention
    bOverlapCorrected = PreventOverlap(LeftFootPos, RightFootPos, RightAxis, MinSeparation);

    // Calculate actual separation for debugging
    FVector Sep = RightFootPos - LeftFootPos;
    CurrentSeparation = FVector::DotProduct(Sep, RightAxis);

    // Set outputs
    LeftFootTargetPosition = LeftFootPos;
    RightFootTargetPosition = RightFootPos;

    // Debug visualization
    if (bDrawDebug && ExecuteContext.GetDrawInterface() != nullptr)
    {
        FRigVMDrawInterface* DrawInterface = ExecuteContext.GetDrawInterface();
        const FTransform& WorldToGlobal = ExecuteContext.GetToWorldSpaceTransform().Inverse();

        // Original positions (blue)
        DrawInterface->DrawPoint(WorldToGlobal, LeftFootTransform.GetLocation(), 6.0f, FLinearColor::Blue);
        DrawInterface->DrawPoint(WorldToGlobal, RightFootTransform.GetLocation(), 6.0f, FLinearColor::Blue);
        // Target positions before overlap correction (yellow)
        FVector LeftBeforeOverlap = LeftFootTransform.GetLocation() + LeftFootOffset * BlendWeight;
        FVector RightBeforeOverlap = RightFootTransform.GetLocation() + RightFootOffset * BlendWeight;
        DrawInterface->DrawPoint(WorldToGlobal, LeftBeforeOverlap, 4.0f, FLinearColor::Yellow);
        DrawInterface->DrawPoint(WorldToGlobal, RightBeforeOverlap, 4.0f, FLinearColor::Yellow);

        // Final positions (green)
        DrawInterface->DrawPoint(WorldToGlobal, LeftFootTargetPosition, 10.0f, FLinearColor::Green);
        DrawInterface->DrawPoint(WorldToGlobal, RightFootTargetPosition, 10.0f, FLinearColor::Green);

        // Center line
        FVector CenterPos = (LeftFootTargetPosition + RightFootTargetPosition) * 0.5f;
        DrawInterface->DrawLine(WorldToGlobal, CenterPos - RightAxis * 50.0f, CenterPos + RightAxis * 50.0f,
            FLinearColor::White, 1.0f);

        // Overlap correction indicator
        if (bOverlapCorrected)
        {
            DrawInterface->DrawLine(WorldToGlobal, LeftFootTargetPosition, RightFootTargetPosition, 
                FLinearColor::Red, 2.0f);
        }
    }
}

// ============================================================================
// FFVRigUnit_LegStepWidthSimple
// ============================================================================

FRigVMStructUpgradeInfo FFVRigUnit_LegStepWidthSimple::GetUpgradeInfo() const
{
    return FRigVMStructUpgradeInfo();
}

FFVRigUnit_LegStepWidthSimple_Execute()
{
    using namespace FVLegStepWidthUtils;
    
    if (BlendWeight <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
    if (Hierarchy == nullptr)
    {
        return;
    }

    // Validate inputs
    if (!LeftFootIKControl.IsValid() || !RightFootIKControl.IsValid() || !PelvisReference.IsValid())
    {
        return;
    }

    const int32 LeftFootIdx = Hierarchy->GetIndex(LeftFootIKControl);
    const int32 RightFootIdx = Hierarchy->GetIndex(RightFootIKControl);
    const int32 PelvisIdx = Hierarchy->GetIndex(PelvisReference);

    if (LeftFootIdx == INDEX_NONE || RightFootIdx == INDEX_NONE || PelvisIdx == INDEX_NONE)
    {
        return;
    }

    // Get transforms
    FTransform LeftFootTransform = Hierarchy->GetGlobalTransform(LeftFootIdx);
    FTransform RightFootTransform = Hierarchy->GetGlobalTransform(RightFootIdx);
    FTransform PelvisTransform = Hierarchy->GetGlobalTransform(PelvisIdx);

    // Get horizontal right axis
    FVector RightAxis = PelvisTransform.GetRotation().GetRightVector();
    RightAxis.Z = 0.0f;
    RightAxis.Normalize();

    // Calculate offsets directly (no spring physics)
    FVector LeftOffset = CalculateLateralOffset(StepWidth, LeftPlantWeight, true, RightAxis);
    FVector RightOffset = CalculateLateralOffset(StepWidth, RightPlantWeight, false, RightAxis);

    // Apply offsets
    FVector LeftPos = LeftFootTransform.GetLocation() + LeftOffset * BlendWeight;
    FVector RightPos = RightFootTransform.GetLocation() + RightOffset * BlendWeight;

    // Overlap prevention
    PreventOverlap(LeftPos, RightPos, RightAxis, MinSeparation);

    // Set outputs
    LeftFootTargetPosition = LeftPos;
    RightFootTargetPosition = RightPos;
}
