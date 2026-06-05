// Copyright FlickerVoid. All Rights Reserved.

#include "Animation/FVLocomotionStyleData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLocomotionStyleData)

UFVLocomotionStyleData::UFVLocomotionStyleData()
{
    // Default values represent a neutral/masculine walk
    // Override these in derived data assets
}

UFVLocomotionStyleData* UFVLocomotionStyleData::CreateBlended(
    const UFVLocomotionStyleData* A,
    const UFVLocomotionStyleData* B,
    float Alpha)
{
    if (A == nullptr || B == nullptr)
    {
        return nullptr;
    }

    // Create a transient object for the blended result
    UFVLocomotionStyleData* Result = NewObject<UFVLocomotionStyleData>();
    
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    // Hip
    Result->HipSwingAmount = FMath::Lerp(A->HipSwingAmount, B->HipSwingAmount, Alpha);
    Result->HipLateralSwing = FMath::Lerp(A->HipLateralSwing, B->HipLateralSwing, Alpha);
    Result->HipVerticalBob = FMath::Lerp(A->HipVerticalBob, B->HipVerticalBob, Alpha);
    Result->HipRotation = FMath::Lerp(A->HipRotation, B->HipRotation, Alpha);

    // Spine
    Result->SpineBendAmount = FMath::Lerp(A->SpineBendAmount, B->SpineBendAmount, Alpha);
    Result->SpineCompensation = FMath::Lerp(A->SpineCompensation, B->SpineCompensation, Alpha);
    Result->SpineLean = FMath::Lerp(A->SpineLean, B->SpineLean, Alpha);
    // Spine curve: use A's if Alpha < 0.5, else B's
    Result->SpineShapeCurve = (Alpha < 0.5f) ? A->SpineShapeCurve : B->SpineShapeCurve;

    // Arms
    Result->ArmSwingAmount = FMath::Lerp(A->ArmSwingAmount, B->ArmSwingAmount, Alpha);
    Result->ArmStiffness = FMath::Lerp(A->ArmStiffness, B->ArmStiffness, Alpha);
    Result->ArmAsymmetry = FMath::Lerp(A->ArmAsymmetry, B->ArmAsymmetry, Alpha);

    // Legs
    Result->StepWidth = FMath::Lerp(A->StepWidth, B->StepWidth, Alpha);
    Result->MinStepSeparation = FMath::Lerp(A->MinStepSeparation, B->MinStepSeparation, Alpha);
    Result->LegStiffness = FMath::Lerp(A->LegStiffness, B->LegStiffness, Alpha);
    Result->KneeBend = FMath::Lerp(A->KneeBend, B->KneeBend, Alpha);

    // Global
    Result->Randomness = FMath::Lerp(A->Randomness, B->Randomness, Alpha);
    Result->EnergyLevel = FMath::Lerp(A->EnergyLevel, B->EnergyLevel, Alpha);

    return Result;
}

void UFVLocomotionStyleData::GetParametersForRig(
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
    float& OutRandomness) const
{
    // Apply energy scaling where appropriate
    float EnergyScale = FMath::Clamp(EnergyLevel, 0.3f, 2.0f);
    
    OutHipSwing = HipSwingAmount * FMath::Lerp(0.8f, 1.2f, EnergyScale - 0.5f);
    OutHipLateral = HipLateralSwing * EnergyScale;
    OutHipVertical = HipVerticalBob * EnergyScale;
    OutSpineBend = SpineBendAmount;
    OutSpineCompensation = SpineCompensation;
    OutArmSwing = ArmSwingAmount * EnergyScale;
    OutArmStiffness = ArmStiffness;
    OutArmAsymmetry = ArmAsymmetry;
    OutStepWidth = StepWidth;
    OutMinSeparation = MinStepSeparation;
    OutLegStiffness = LegStiffness;
    OutRandomness = Randomness;
}
