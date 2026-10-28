#include "PoseAuthoring/FVPoseAuthoringLibrary.h"

#include "Animation/PoseAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "PoseAuthoring/FVPoseAuthoring.h"
#include "PoseAuthoring/FVPoseMirror.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPoseAuthoringLibrary)

#define LOCTEXT_NAMESPACE "FVPoseAuthoringLibrary"

namespace
{
	bool CheckAsset(const UPoseAsset* PoseAsset, FText& OutError)
	{
		if (!PoseAsset)
		{
			OutError = LOCTEXT("ErrNoAsset", "No Pose Asset given.");
			return false;
		}
		return true;
	}

	bool CheckSource(const USkeletalMeshComponent* Source, FText& OutError)
	{
		if (!Source)
		{
			OutError = LOCTEXT("ErrNoSource", "No mesh component given.");
			return false;
		}
		return true;
	}
}

bool UFVPoseAuthoringLibrary::CreatePose(UPoseAsset* PoseAsset, FName Pose, USkeletalMeshComponent* Source, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && CheckSource(Source, OutError) && FFVPoseAuthoring::CreatePose(*PoseAsset, Pose, *Source, OutError);
}

bool UFVPoseAuthoringLibrary::UpdatePose(UPoseAsset* PoseAsset, FName Pose, USkeletalMeshComponent* Source, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && CheckSource(Source, OutError) && FFVPoseAuthoring::UpdatePose(*PoseAsset, Pose, *Source, OutError);
}

bool UFVPoseAuthoringLibrary::RenamePose(UPoseAsset* PoseAsset, FName Pose, FName NewName, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseAuthoring::RenamePose(*PoseAsset, Pose, NewName, OutError);
}

bool UFVPoseAuthoringLibrary::DeletePoses(UPoseAsset* PoseAsset, const TArray<FName>& Poses, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseAuthoring::DeletePoses(*PoseAsset, Poses, OutError);
}

bool UFVPoseAuthoringLibrary::DuplicatePose(UPoseAsset* PoseAsset, FName Pose, FName NewName, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseAuthoring::DuplicatePose(*PoseAsset, Pose, NewName, OutError);
}

TArray<FName> UFVPoseAuthoringLibrary::GetChangedBones(UPoseAsset* PoseAsset, FName Pose)
{
	return PoseAsset ? FFVPoseAuthoring::GetChangedBones(*PoseAsset, Pose) : TArray<FName>();
}

bool UFVPoseAuthoringLibrary::CanMirrorPose(UPoseAsset* PoseAsset, FName Pose, FText& OutReason)
{
	return CheckAsset(PoseAsset, OutReason) && FFVPoseMirror::CanMirrorPose(*PoseAsset, Pose, OutReason);
}

FName UFVPoseAuthoringLibrary::GetMirrorPoseName(FName Pose)
{
	return FFVPoseMirror::GetMirrorPoseName(Pose);
}

bool UFVPoseAuthoringLibrary::MirrorPoses(UPoseAsset* PoseAsset, const TArray<FName>& Poses, EFVMirrorAxis Axis, TArray<FName>& OutMirroredPoses, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseMirror::MirrorPoses(*PoseAsset, Poses, Axis, OutMirroredPoses, OutError);
}

#undef LOCTEXT_NAMESPACE
