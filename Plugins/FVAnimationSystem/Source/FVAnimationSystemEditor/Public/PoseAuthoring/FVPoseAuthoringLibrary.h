#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVPoseAuthoringLibrary.generated.h"

class UPoseAsset;
class USkeletalMeshComponent;

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
};
