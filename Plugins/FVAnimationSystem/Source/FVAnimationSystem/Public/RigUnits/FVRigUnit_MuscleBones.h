#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "FVRigUnit_MuscleBones.generated.h"

/** A muscle bone that aims at its target bone and stretches to it, thickening as it shortens. */
USTRUCT(BlueprintType)
struct FVANIMATIONSYSTEM_API FFVMuscleStretch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Muscle", meta = (CustomWidget = "BoneName", ToolTip = "The muscle's belly bone (mb_), a child of the origin bone."))
	FName Bone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Muscle", meta = (CustomWidget = "BoneName", ToolTip = "The muscle's target bone (mt_), a child of the insertion bone."))
	FName Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Muscle", meta = (ClampMin = "0.0", ToolTip = "How much the bone thickens as it shortens; 1 keeps its volume."))
	float Volume = 1.0f;
};

/** A joint helper bone that turns part of the way with another bone, holding volume in a fold. */
USTRUCT(BlueprintType)
struct FVANIMATIONSYSTEM_API FFVJointFollow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint", meta = (CustomWidget = "BoneName", ToolTip = "The half bone (jh_) at the joint, a child of the joint's parent bone."))
	FName Bone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint", meta = (CustomWidget = "BoneName", ToolTip = "The bone it turns with, such as calf_l for a knee."))
	FName Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "How far it turns with the source bone; 0.5 is halfway."))
	float Weight = 0.5f;
};

/**
 * Drives the Meatbag Factory muscle and joint bones made in Blender, the same way Blender's Stretch To and Copy
 * Rotation constraints do, from the reference pose. Place it after the animation and IK, before correctives.
 */
USTRUCT(meta = (DisplayName = "FV Muscle Bones", Category = "FlickerVoid|Animation", Keywords = "Muscle,Helper,Corrective,Stretch,Knee,Elbow,Meatbag"))
struct FVANIMATIONSYSTEM_API FFVRigUnit_MuscleBones : public FRigUnitMutable
{
	GENERATED_BODY()

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta = (Input, ToolTip = "Muscle bones and the target bones they stretch to."))
	TArray<FFVMuscleStretch> Muscles;

	UPROPERTY(meta = (Input, ToolTip = "Knee and elbow half bones and the bones they turn with."))
	TArray<FFVJointFollow> Joints;

	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Blend from the animated pose (0) to the driven bones (1)."))
	float Alpha = 1.0f;
};
