#include "PoseAuthoring/FVPoseAuthoringLibrary.h"

#include "Animation/AnimSequence.h"
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

bool UFVPoseAuthoringLibrary::MirrorAll(UPoseAsset* PoseAsset, EFVBodySide From, EFVMirrorAxis Axis, bool bDeleteOrphans, TArray<FName>& OutMirroredPoses, TArray<FName>& OutSkippedPoses, TArray<FName>& OutOrphans, FText& OutError)
{
	FFVMirrorAllResult Result;
	const bool bDone = CheckAsset(PoseAsset, OutError) && FFVPoseMirror::MirrorAll(*PoseAsset, From, Axis, bDeleteOrphans, Result, OutError);
	OutMirroredPoses = Result.Created;
	OutMirroredPoses.Append(Result.Updated);
	OutSkippedPoses.Reset();
	for (const TPair<FName, FText>& Skipped : Result.Skipped)
	{
		OutSkippedPoses.Add(Skipped.Key);
	}
	OutOrphans = Result.Orphans;
	return bDone;
}

bool UFVPoseAuthoringLibrary::CreateInbetweens(UPoseAsset* PoseAsset, FName Pose, int32 Count, TArray<FName>& OutPoses, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseAuthoring::CreateInbetweens(*PoseAsset, Pose, Count, OutPoses, OutError);
}

TArray<FName> UFVPoseAuthoringLibrary::GetAddableCurves(UPoseAsset* PoseAsset)
{
	return PoseAsset ? FFVPoseAuthoring::GetAddableCurves(*PoseAsset) : TArray<FName>();
}

bool UFVPoseAuthoringLibrary::AddCurve(UPoseAsset* PoseAsset, FName Curve, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseAuthoring::AddCurve(*PoseAsset, Curve, OutError);
}

bool UFVPoseAuthoringLibrary::SetCurveValue(UPoseAsset* PoseAsset, FName Pose, FName Curve, float Value, FText& OutError)
{
	return CheckAsset(PoseAsset, OutError) && FFVPoseAuthoring::SetCurveValue(*PoseAsset, Pose, Curve, Value, OutError);
}

UPoseAsset* UFVPoseAuthoringLibrary::CreatePoseAsset(const FString& AssetPath, USkeletalMesh* Mesh, FText& OutError)
{
	if (!Mesh)
	{
		OutError = LOCTEXT("ErrNoMesh", "No skeletal mesh given.");
		return nullptr;
	}
	return FFVPoseAuthoring::CreatePoseAsset(AssetPath, *Mesh, OutError);
}

FFVPoseImportResult UFVPoseAuthoringLibrary::SetPosesFromAnimation(UPoseAsset* PoseAsset, UAnimSequence* Animation, const TArray<FName>& Poses)
{
	FFVPoseImportResult Result;
	if (!Animation)
	{
		Result.Error = LOCTEXT("ErrNoAnimation", "No animation given.");
		return Result;
	}
	Result.bSucceeded = CheckAsset(PoseAsset, Result.Error)
		&& FFVPoseAuthoring::SetPosesFromAnimation(*PoseAsset, *Animation, Poses, Result.Created, Result.Updated, Result.Error);
	return Result;
}

#undef LOCTEXT_NAMESPACE
