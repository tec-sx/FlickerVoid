#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.h"

class UPoseAsset;
struct FFVPoseSnapshot;

/**
 * Mirrors Pose Asset poses onto their counterparts. A pose's side comes from the side token in its name
 * (upperarm_l_up_90); only the bones with that side's suffix are mirrored, and centre or opposite-side
 * bones are never touched. The counterpart pose is updated when it exists and created when it doesn't.
 */
struct FVANIMATIONSYSTEMEDITOR_API FFVPoseMirror
{
	/** False with the reason when the pose can't be mirrored. */
	static bool CanMirrorPose(const UPoseAsset& Asset, FName Pose, FText& OutReason);

	/** The counterpart pose name; None when the name has no single side. */
	static FName GetMirrorPoseName(FName Pose);

	/** The counterpart pose as it would be written, without changing the asset. */
	static bool BuildMirroredPose(const UPoseAsset& Asset, FName Pose, EFVMirrorAxis Axis, FName& OutTarget, FFVPoseSnapshot& OutTargetPose, FText& OutError);

	/**
	 * Mirrors several poses as one undo step. Everything is checked first and nothing is written when any pose
	 * fails, when a pose and its counterpart are both selected, or when two poses share a counterpart.
	 */
	static bool MirrorPoses(UPoseAsset& Asset, const TArray<FName>& Poses, EFVMirrorAxis Axis, TArray<FName>& OutTargets, FText& OutError);
};
