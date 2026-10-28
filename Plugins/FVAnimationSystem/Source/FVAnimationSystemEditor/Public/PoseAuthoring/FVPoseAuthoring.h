#pragma once

#include "CoreMinimal.h"

class UPoseAsset;
class USkeletalMesh;
class USkeletalMeshComponent;

/** One pose of a Pose Asset as full local-space bone transforms and curve values, keyed by name. */
struct FVANIMATIONSYSTEMEDITOR_API FFVPoseSnapshot
{
	TMap<FName, FTransform> Bones;
	TMap<FName, float> Curves;
};

/**
 * Pose Asset commands behind the Meatbag Factory. Editor game thread only. Commands that change the asset
 * are one undo step each and return false with OutError, leaving the asset unchanged, when they refuse.
 */
struct FVANIMATIONSYSTEMEDITOR_API FFVPoseAuthoring
{
	static bool ReadPose(const UPoseAsset& Asset, FName Pose, FFVPoseSnapshot& OutPose);

	/** The reference pose, or the base pose of an additive asset. */
	static FFVPoseSnapshot ReadBasePose(const UPoseAsset& Asset);

	/** Bones whose transform differs from the base pose by more than the settings' tolerances. */
	static TArray<FName> GetChangedBones(const UPoseAsset& Asset, FName Pose);

	/** The mesh used to write computed poses: the asset's preview mesh, else its skeleton's. */
	static USkeletalMesh* FindMesh(const UPoseAsset& Asset);

	static bool CreatePose(UPoseAsset& Asset, FName Pose, const USkeletalMeshComponent& Source, FText& OutError);
	static bool UpdatePose(UPoseAsset& Asset, FName Pose, const USkeletalMeshComponent& Source, FText& OutError);
	static bool RenamePose(UPoseAsset& Asset, FName Pose, FName NewName, FText& OutError);
	static bool DeletePoses(UPoseAsset& Asset, const TArray<FName>& Poses, FText& OutError);
	static bool DuplicatePose(UPoseAsset& Asset, FName Pose, FName NewName, FText& OutError);

	/** Adds or replaces a pose from computed data, without opening a transaction. Bones missing from Pose keep the mesh reference pose. */
	static bool WritePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError);
};
