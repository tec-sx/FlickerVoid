#pragma once

#include "CoreMinimal.h"
#include "Rigs/RigHierarchyCache.h"
#include "Units/RigUnit.h"
#include "FVRigUnit_MaintainVolume.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "MaintainVolumeMode"))
enum class EFVMaintainVolumeMode : uint8
{
	Strict UMETA(ToolTip = "The other two axes are set from the free axis alone, overriding their own scale."),
	Uniform UMETA(ToolTip = "Volume holds against the combined scale of all three axes; uniform scaling keeps the volume."),
	SingleAxis UMETA(DisplayName = "Single Axis", ToolTip = "Volume holds against the free axis; the other axes keep their own scale on top.")
};

UENUM(BlueprintType, meta = (ScriptName = "VolumeAxis"))
enum class EFVVolumeAxis : uint8
{
	X UMETA(ToolTip = "Bone X, the length axis of UE Mannequin bones."),
	Y UMETA(ToolTip = "Bone Y, the length axis of Blender bones."),
	Z UMETA(ToolTip = "Bone Z.")
};

/**
 * Keeps a bone's volume while its free axis scales, like Blender's Maintain Volume constraint in local space:
 * the other two axes scale by the square root of Volume over the free axis scale. Scale is measured against the
 * reference pose. Place it after whatever scales the bone (animation, Pose Asset, stretch).
 */
USTRUCT(meta = (DisplayName = "FV Maintain Volume", Category = "FlickerVoid|Animation", Keywords = "Volume,Squash,Stretch,Bulge,Scale,Muscle,Corrective"))
struct FVANIMATIONSYSTEM_API FFVRigUnit_MaintainVolume : public FRigUnitMutable
{
	GENERATED_BODY()

	RIGVM_METHOD()
	virtual void Execute() override;

	UPROPERTY(meta = (Input, ExpandByDefault, ToolTip = "The bone whose volume is kept."))
	FRigElementKey Item;

	UPROPERTY(meta = (Input, ToolTip = "How the other two axes follow the free axis."))
	EFVMaintainVolumeMode Mode = EFVMaintainVolumeMode::Strict;

	UPROPERTY(meta = (Input, ToolTip = "The axis that scales freely; the other two compensate."))
	EFVVolumeAxis FreeAxis = EFVVolumeAxis::X;

	UPROPERTY(meta = (Input, ClampMin = "0.001", ToolTip = "Volume to keep, relative to the reference pose; 1 keeps it, above 1 bulges."))
	float Volume = 1.0f;

	UPROPERTY(meta = (Input, ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Blend from the incoming scale (0) to the volume-kept scale (1)."))
	float Alpha = 1.0f;

	UPROPERTY(meta = (Input, ToolTip = "Move the bone's children with it; off keeps their global transforms."))
	bool bPropagateToChildren = true;

	UPROPERTY()
	FCachedRigElement CachedItem;
};
