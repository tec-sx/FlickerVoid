#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.h"

class UPoseAsset;
struct FFVPoseSnapshot;

/** What Mirror All did, or would do, to a Pose Asset. */
struct FVANIMATIONSYSTEMEDITOR_API FFVMirrorAllResult
{
	/** Counterpart poses that didn't exist. */
	TArray<FName> Created;
	/** Counterpart poses that existed and were overwritten. */
	TArray<FName> Updated;
	/** Authoring-side poses left alone, with the reason. */
	TArray<TPair<FName, FText>> Skipped;
	/** Other-side poses whose authoring-side pose no longer exists (renamed or deleted). */
	TArray<FName> Orphans;
	bool bOrphansDeleted = false;
};

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

	/** What Mirror All from the From side would create, update, skip and orphan, without changing the asset. */
	static FFVMirrorAllResult PlanMirrorAll(const UPoseAsset& Asset, EFVBodySide From);

	/**
	 * Mirrors every From-side pose that can be mirrored, and deletes the orphans when bDeleteOrphans, as one undo step.
	 * Skipped poses don't fail the call; it fails only when there is nothing to do or a write fails.
	 */
	static bool MirrorAll(UPoseAsset& Asset, EFVBodySide From, EFVMirrorAxis Axis, bool bDeleteOrphans, FFVMirrorAllResult& OutResult, FText& OutError);
};
