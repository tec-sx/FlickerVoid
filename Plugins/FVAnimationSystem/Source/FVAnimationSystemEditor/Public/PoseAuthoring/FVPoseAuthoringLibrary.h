#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVPoseAuthoringLibrary.generated.h"

class UAnimSequence;
class UPoseAsset;
class USkeletalMesh;
class USkeletalMeshComponent;

/** What SetPosesFromAnimation did, or why it refused. A struct rather than a bool so Python gets the error too. */
USTRUCT(BlueprintType)
struct FVANIMATIONSYSTEMEDITOR_API FFVPoseImportResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FV|Animation|Meatbag Factory")
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadOnly, Category = "FV|Animation|Meatbag Factory")
	TArray<FName> Created;

	UPROPERTY(BlueprintReadOnly, Category = "FV|Animation|Meatbag Factory")
	TArray<FName> Updated;

	UPROPERTY(BlueprintReadOnly, Category = "FV|Animation|Meatbag Factory")
	FText Error;
};

/** Editor scripting for the Meatbag Factory: name-keyed Pose Asset commands, each one undo step. */
UCLASS()
class FVANIMATIONSYSTEMEDITOR_API UFVPoseAuthoringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Adds a pose from the current pose of Source, whose mesh uses the asset's skeleton."))
	static bool CreatePose(UPoseAsset* PoseAsset, FName Pose, USkeletalMeshComponent* Source, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Replaces an existing pose with the current pose of Source."))
	static bool UpdatePose(UPoseAsset* PoseAsset, FName Pose, USkeletalMeshComponent* Source, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory")
	static bool RenamePose(UPoseAsset* PoseAsset, FName Pose, FName NewName, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Deletes all the poses or none, when one doesn't exist."))
	static bool DeletePoses(UPoseAsset* PoseAsset, const TArray<FName>& Poses, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory")
	static bool DuplicatePose(UPoseAsset* PoseAsset, FName Pose, FName NewName, FText& OutError);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Bones the pose moves away from the base pose, beyond the settings' tolerances."))
	static TArray<FName> GetChangedBones(UPoseAsset* PoseAsset, FName Pose);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "False with the reason when the pose can't be mirrored."))
	static bool CanMirrorPose(UPoseAsset* PoseAsset, FName Pose, FText& OutReason);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "The counterpart pose name, such as upperarm_r_up for upperarm_l_up; None without a single side token."))
	static FName GetMirrorPoseName(FName Pose);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Mirrors the poses' side bones onto their counterpart poses, updating or creating them, as one undo step. All or nothing."))
	static bool MirrorPoses(UPoseAsset* PoseAsset, const TArray<FName>& Poses, EFVMirrorAxis Axis, TArray<FName>& OutMirroredPoses, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Mirrors every pose of the From side that can be mirrored, and deletes the other side's orphans (poses whose From-side pose is gone) when Delete Orphans is set. One undo step."))
	static bool MirrorAll(UPoseAsset* PoseAsset, EFVBodySide From, EFVMirrorAxis Axis, bool bDeleteOrphans, TArray<FName>& OutMirroredPoses, TArray<FName>& OutSkippedPoses, TArray<FName>& OutOrphans, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Adds Count (1 to 9) poses between the base pose and Pose, named <Pose>_ib<percent>."))
	static bool CreateInbetweens(UPoseAsset* PoseAsset, FName Pose, int32 Count, TArray<FName>& OutPoses, FText& OutError);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Skeleton curves (Skeleton > Anim Curves) the Pose Asset doesn't have yet."))
	static TArray<FName> GetAddableCurves(UPoseAsset* PoseAsset);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Adds a skeleton curve at 0 on every pose, keeping the poses, their names and the additive setup."))
	static bool AddCurve(UPoseAsset* PoseAsset, FName Curve, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory")
	static bool SetCurveValue(UPoseAsset* PoseAsset, FName Pose, FName Curve, float Value, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Creates an empty, unsaved Pose Asset for the mesh's skeleton at Asset Path (/Game/Folder/PA_Name), with the mesh as preview mesh. None with the error when it refuses."))
	static UPoseAsset* CreatePoseAsset(const FString& AssetPath, USkeletalMesh* Mesh, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "FV|Animation|Meatbag Factory", meta = (ToolTip = "Frame i of Animation becomes pose Poses[i]: updated when the asset has it (keeping its curve values), created otherwise. One undo step, all or nothing. Used by the Blender tools to send poses."))
	static FFVPoseImportResult SetPosesFromAnimation(UPoseAsset* PoseAsset, UAnimSequence* Animation, const TArray<FName>& Poses);
};
