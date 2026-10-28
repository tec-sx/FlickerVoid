#include "PoseAuthoring/FVPoseMirror.h"

#include "AnimationRuntime.h"
#include "Animation/PoseAsset.h"
#include "Engine/SkeletalMesh.h"
#include "FVAnimationSettings.h"
#include "FVMirrorNameRule.h"
#include "PoseAuthoring/FVPoseAuthoring.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "FVPoseMirror"

namespace
{
	EAxis::Type ResolveAxis(EFVMirrorAxis Axis)
	{
		switch (Axis)
		{
		case EFVMirrorAxis::X: return EAxis::X;
		case EFVMirrorAxis::Y: return EAxis::Y;
		case EFVMirrorAxis::Z: return EAxis::Z;
		default:
			{
				const EAxis::Type Default = UFVAnimationSettings::Get().DefaultMirrorAxis;
				return Default == EAxis::None ? EAxis::X : Default;
			}
		}
	}

	/** Component-space reference rotation of every bone. */
	TArray<FQuat> GetComponentSpaceRefRotations(const FReferenceSkeleton& RefSkeleton)
	{
		const TArray<FTransform>& RefPose = RefSkeleton.GetRefBonePose();
		TArray<FQuat> Rotations;
		Rotations.SetNum(RefSkeleton.GetNum());
		for (int32 Bone = 0; Bone < RefSkeleton.GetNum(); ++Bone)
		{
			const int32 Parent = RefSkeleton.GetParentIndex(Bone);
			Rotations[Bone] = Parent == INDEX_NONE ? RefPose[Bone].GetRotation() : Rotations[Parent] * RefPose[Bone].GetRotation();
		}
		return Rotations;
	}

	/** The local transform of Source mirrored onto Target; the same rule as FAnimationRuntime::MirrorPose. */
	FTransform MirrorBone(const FTransform& Local, int32 Source, int32 Target, const FReferenceSkeleton& RefSkeleton, const TArray<FQuat>& RefRotations, EAxis::Type Axis)
	{
		const int32 SourceParent = RefSkeleton.GetParentIndex(Source);
		const int32 TargetParent = RefSkeleton.GetParentIndex(Target);
		const FQuat SourceParentRotation = SourceParent == INDEX_NONE ? FQuat::Identity : RefRotations[SourceParent];
		const FQuat TargetParentRotation = TargetParent == INDEX_NONE ? FQuat::Identity : RefRotations[TargetParent];

		FVector Translation = SourceParentRotation.RotateVector(Local.GetTranslation());
		Translation = FAnimationRuntime::MirrorVector(Translation, Axis);
		Translation = TargetParentRotation.UnrotateVector(Translation);

		FQuat Rotation = FAnimationRuntime::MirrorQuat(SourceParentRotation * Local.GetRotation(), Axis);
		Rotation *= FAnimationRuntime::MirrorQuat(RefRotations[Source], Axis).Inverse() * RefRotations[Target];
		Rotation = TargetParentRotation.Inverse() * Rotation;

		return FTransform(Rotation.GetNormalized(), Translation, Local.GetScale3D());
	}
}

FName FFVPoseMirror::GetMirrorPoseName(FName Pose)
{
	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	const FString Name = Pose.ToString();
	return Rule.GetNameSide(Name) == EFVBodySide::Centre ? NAME_None : FName(*Rule.MirrorName(Name));
}

bool FFVPoseMirror::CanMirrorPose(const UPoseAsset& Asset, FName Pose, FText& OutReason)
{
	if (!Asset.ContainsPose(Pose))
	{
		OutReason = FText::Format(LOCTEXT("ErrNoPose", "There is no pose named {0}."), FText::FromName(Pose));
		return false;
	}

	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	const EFVBodySide Side = Rule.GetNameSide(Pose.ToString());
	if (Side == EFVBodySide::Centre)
	{
		OutReason = FText::Format(LOCTEXT("ErrNoSide", "{0} has no single side token, such as _l or _r, in its name."), FText::FromName(Pose));
		return false;
	}

	const USkeletalMesh* Mesh = FFVPoseAuthoring::FindMesh(Asset);
	if (!Mesh)
	{
		OutReason = LOCTEXT("ErrNoMesh", "Set a preview mesh on the Pose Asset or its skeleton.");
		return false;
	}

	bool bHasSideBone = false;
	for (const FName Bone : FFVPoseAuthoring::GetChangedBones(Asset, Pose))
	{
		if (Rule.GetBoneSide(Bone.ToString()) != Side)
		{
			continue;
		}
		bHasSideBone = true;
		const FString Counterpart = Rule.MirrorBoneName(Bone.ToString());
		if (Mesh->GetRefSkeleton().FindBoneIndex(FName(*Counterpart)) == INDEX_NONE)
		{
			OutReason = FText::Format(LOCTEXT("ErrNoCounterpart", "{0} has no counterpart {1} in the skeleton."), FText::FromName(Bone), FText::FromString(Counterpart));
			return false;
		}
	}
	if (!bHasSideBone)
	{
		OutReason = FText::Format(LOCTEXT("ErrCentreOnly", "{0} moves no bones with its side suffix; centre bones are never mirrored."), FText::FromName(Pose));
		return false;
	}
	return true;
}

bool FFVPoseMirror::BuildMirroredPose(const UPoseAsset& Asset, FName Pose, EFVMirrorAxis Axis, FName& OutTarget, FFVPoseSnapshot& OutTargetPose, FText& OutError)
{
	FFVPoseSnapshot Source;
	if (!CanMirrorPose(Asset, Pose, OutError) || !FFVPoseAuthoring::ReadPose(Asset, Pose, Source))
	{
		return false;
	}

	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	const EFVBodySide Side = Rule.GetNameSide(Pose.ToString());
	OutTarget = GetMirrorPoseName(Pose);
	if (!FFVPoseAuthoring::ReadPose(Asset, OutTarget, OutTargetPose))
	{
		OutTargetPose = FFVPoseAuthoring::ReadBasePose(Asset);
	}

	const FReferenceSkeleton& RefSkeleton = FFVPoseAuthoring::FindMesh(Asset)->GetRefSkeleton();
	const TArray<FQuat> RefRotations = GetComponentSpaceRefRotations(RefSkeleton);
	const EAxis::Type MirrorAxis = ResolveAxis(Axis);
	for (const TPair<FName, FTransform>& Bone : Source.Bones)
	{
		if (Rule.GetBoneSide(Bone.Key.ToString()) != Side)
		{
			continue;
		}
		const FName Counterpart(*Rule.MirrorBoneName(Bone.Key.ToString()));
		const int32 SourceIndex = RefSkeleton.FindBoneIndex(Bone.Key);
		const int32 TargetIndex = RefSkeleton.FindBoneIndex(Counterpart);
		if (SourceIndex != INDEX_NONE && TargetIndex != INDEX_NONE)
		{
			OutTargetPose.Bones.Add(Counterpart, MirrorBone(Bone.Value, SourceIndex, TargetIndex, RefSkeleton, RefRotations, MirrorAxis));
		}
	}

	for (const TPair<FName, float>& Curve : Source.Curves)
	{
		if (Rule.GetNameSide(Curve.Key.ToString()) != Side)
		{
			continue;
		}
		if (float* TargetValue = OutTargetPose.Curves.Find(FName(*Rule.MirrorName(Curve.Key.ToString()))))
		{
			*TargetValue = Curve.Value;
		}
	}
	return true;
}

bool FFVPoseMirror::MirrorPoses(UPoseAsset& Asset, const TArray<FName>& Poses, EFVMirrorAxis Axis, TArray<FName>& OutTargets, FText& OutError)
{
	OutTargets.Reset();
	const TArray<FName> Unique = TSet<FName>(Poses).Array();
	if (Unique.IsEmpty())
	{
		OutError = LOCTEXT("ErrNoSelection", "No poses selected.");
		return false;
	}

	TArray<FFVPoseSnapshot> Mirrored;
	for (const FName Pose : Unique)
	{
		FName Target;
		FFVPoseSnapshot TargetPose;
		if (!BuildMirroredPose(Asset, Pose, Axis, Target, TargetPose, OutError))
		{
			return false;
		}
		if (Unique.Contains(Target))
		{
			OutError = FText::Format(LOCTEXT("ErrBothSelected", "{0} and its counterpart {1} are both selected."), FText::FromName(Pose), FText::FromName(Target));
			return false;
		}
		if (OutTargets.Contains(Target))
		{
			OutError = FText::Format(LOCTEXT("ErrSharedTarget", "Two selected poses mirror onto {0}."), FText::FromName(Target));
			return false;
		}
		OutTargets.Add(Target);
		Mirrored.Add(MoveTemp(TargetPose));
	}

	FScopedTransaction Transaction(LOCTEXT("MirrorPoses", "Mirror Poses"));
	Asset.Modify();
	for (int32 Index = 0; Index < OutTargets.Num(); ++Index)
	{
		if (!FFVPoseAuthoring::WritePose(Asset, OutTargets[Index], Mirrored[Index], OutError))
		{
			Transaction.Cancel();
			OutTargets.Reset();
			return false;
		}
	}
	return true;
}

FFVMirrorAllResult FFVPoseMirror::PlanMirrorAll(const UPoseAsset& Asset, EFVBodySide From)
{
	FFVMirrorAllResult Result;
	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	const EFVBodySide To = From == EFVBodySide::Left ? EFVBodySide::Right : EFVBodySide::Left;
	for (const FName Pose : Asset.GetPoseFNames())
	{
		const EFVBodySide Side = Rule.GetNameSide(Pose.ToString());
		if (Side == From)
		{
			FText Reason;
			if (!CanMirrorPose(Asset, Pose, Reason))
			{
				Result.Skipped.Emplace(Pose, Reason);
			}
			else
			{
				(Asset.ContainsPose(GetMirrorPoseName(Pose)) ? Result.Updated : Result.Created).Add(GetMirrorPoseName(Pose));
			}
		}
		else if (Side == To && !Asset.ContainsPose(GetMirrorPoseName(Pose)))
		{
			Result.Orphans.Add(Pose);
		}
	}
	return Result;
}

bool FFVPoseMirror::MirrorAll(UPoseAsset& Asset, EFVBodySide From, EFVMirrorAxis Axis, bool bDeleteOrphans, FFVMirrorAllResult& OutResult, FText& OutError)
{
	if (From == EFVBodySide::Centre)
	{
		OutError = LOCTEXT("ErrCentreSide", "Mirror All needs a left or right authoring side.");
		return false;
	}

	OutResult = PlanMirrorAll(Asset, From);
	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	TArray<FName> Sources;
	for (const FName Target : OutResult.Created)
	{
		Sources.Add(FName(*Rule.MirrorName(Target.ToString())));
	}
	for (const FName Target : OutResult.Updated)
	{
		Sources.Add(FName(*Rule.MirrorName(Target.ToString())));
	}
	const bool bDeletes = bDeleteOrphans && !OutResult.Orphans.IsEmpty();
	if (Sources.IsEmpty() && !bDeletes)
	{
		OutError = OutResult.Skipped.IsEmpty()
			? LOCTEXT("ErrNothingToMirror", "There are no poses of the authoring side to mirror.")
			: FText::Format(LOCTEXT("ErrAllSkipped", "None of the {0} authoring-side poses can be mirrored."), OutResult.Skipped.Num());
		return false;
	}

	FScopedTransaction Transaction(LOCTEXT("MirrorAll", "Mirror All"));
	Asset.Modify();
	TArray<FName> Targets;
	if (!Sources.IsEmpty() && !MirrorPoses(Asset, Sources, Axis, Targets, OutError))
	{
		Transaction.Cancel();
		return false;
	}
	if (bDeletes)
	{
		Asset.DeletePoses(OutResult.Orphans);
		OutResult.bOrphansDeleted = true;
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
