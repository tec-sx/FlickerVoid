#pragma once

#include "CoreMinimal.h"

class UAnimSequence;
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
	static bool CreatePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError);
	static bool UpdatePose(UPoseAsset& Asset, FName Pose, const USkeletalMeshComponent& Source, FText& OutError);
	static bool UpdatePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError);
	static bool RenamePose(UPoseAsset& Asset, FName Pose, FName NewName, FText& OutError);
	static bool DeletePoses(UPoseAsset& Asset, const TArray<FName>& Poses, FText& OutError);
	static bool DuplicatePose(UPoseAsset& Asset, FName Pose, FName NewName, FText& OutError);

	/** A pose name not used in the asset: Base, else Base_1, Base_2... */
	static FName MakeUniquePoseName(const UPoseAsset& Asset, const FString& Base);

	/** Count poses between the base pose and Pose, named <Pose>_ib<percent> (upperarm_l_up_ib50). */
	static bool CreateInbetweens(UPoseAsset& Asset, FName Pose, int32 Count, TArray<FName>& OutPoses, FText& OutError);

	/**
	 * Pastes Clipboard onto each target pose. Bones (and curves) move from the base pose towards the clipboard
	 * by Multiplier (1 copies, 0.5 halves, 1.5 exaggerates). An empty Bones set pastes every bone and curve;
	 * otherwise only those bones, and no curves.
	 */
	static bool PastePose(UPoseAsset& Asset, const TArray<FName>& Targets, const FFVPoseSnapshot& Clipboard, const TSet<FName>& Bones, float Multiplier, FText& OutError);

	static bool SetCurveValue(UPoseAsset& Asset, FName Pose, FName Curve, float Value, FText& OutError);

	/** The skeleton's curves (authored in content) that the asset doesn't have yet. */
	static TArray<FName> GetAddableCurves(const UPoseAsset& Asset);

	/** Adds a skeleton curve to the asset, at 0 on every pose, keeping every pose, its name and the additive setup. */
	static bool AddCurve(UPoseAsset& Asset, FName Curve, FText& OutError);

	/** An empty, unsaved Pose Asset at AssetPath (/Game/Folder/PA_Name) for the mesh's skeleton, previewed on the mesh. */
	static UPoseAsset* CreatePoseAsset(const FString& AssetPath, USkeletalMesh& Mesh, FText& OutError);

	/** The animation's local bone transforms at Frame. */
	static FFVPoseSnapshot ReadAnimationFrame(const UAnimSequence& Animation, int32 Frame);

	/** Frame i of Animation becomes pose Poses[i], updated (keeping its curves) or created by name. One undo step, all or nothing. */
	static bool SetPosesFromAnimation(UPoseAsset& Asset, const UAnimSequence& Animation, const TArray<FName>& Poses, TArray<FName>& OutCreated, TArray<FName>& OutUpdated, FText& OutError);

	/** Adds or replaces a pose from computed data, without opening a transaction. Bones missing from Pose keep the mesh reference pose. */
	static bool WritePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError);
};
