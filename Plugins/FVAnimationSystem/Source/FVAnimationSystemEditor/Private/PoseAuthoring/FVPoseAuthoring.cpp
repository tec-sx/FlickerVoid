#include "PoseAuthoring/FVPoseAuthoring.h"

#include "Animation/PoseAsset.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "FVAnimationSettings.h"
#include "PoseAuthoring/FVPoseWriterComponent.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "FVPoseAuthoring"

namespace
{
	bool ValidateNewName(const UPoseAsset& Asset, FName Name, FText& OutError)
	{
		if (Name.IsNone())
		{
			OutError = LOCTEXT("ErrEmptyName", "The pose needs a name.");
			return false;
		}
		if (Asset.ContainsPose(Name))
		{
			OutError = FText::Format(LOCTEXT("ErrNameTaken", "A pose named {0} already exists."), FText::FromName(Name));
			return false;
		}
		return true;
	}

	bool ValidateExisting(const UPoseAsset& Asset, FName Name, FText& OutError)
	{
		if (!Asset.ContainsPose(Name))
		{
			OutError = FText::Format(LOCTEXT("ErrNoPose", "There is no pose named {0}."), FText::FromName(Name));
			return false;
		}
		return true;
	}

	bool ValidateSource(const UPoseAsset& Asset, const USkeletalMeshComponent& Source, FText& OutError)
	{
		const USkeletalMesh* Mesh = Source.GetSkeletalMeshAsset();
		if (!Mesh || Mesh->GetSkeleton() != Asset.GetSkeleton())
		{
			OutError = LOCTEXT("ErrSkeleton", "The mesh must use the Pose Asset's skeleton.");
			return false;
		}
		return true;
	}

	bool IsChanged(const FTransform& A, const FTransform& B, const UFVAnimationSettings& Settings)
	{
		const double Degrees = FMath::RadiansToDegrees(A.GetRotation().AngularDistance(B.GetRotation()));
		return Degrees > Settings.ChangedRotationTolerance
			|| FVector::Dist(A.GetTranslation(), B.GetTranslation()) > Settings.ChangedTranslationTolerance
			|| (A.GetScale3D() - B.GetScale3D()).GetAbsMax() > Settings.ChangedScaleTolerance;
	}
}

bool FFVPoseAuthoring::ReadPose(const UPoseAsset& Asset, FName Pose, FFVPoseSnapshot& OutPose)
{
	const int32 Index = Asset.GetPoseIndexByName(Pose);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	const TArray<FName>& Tracks = Asset.GetTrackNames();
	const TArray<FTransform>& Transforms = Asset.GetFullPose(Index);
	const TArray<FName> Curves = Asset.GetCurveFNames();
	const TArray<float>& Values = Asset.GetFullCurveValues(Index);
	if (Tracks.Num() != Transforms.Num() || Curves.Num() != Values.Num())
	{
		return false;
	}

	OutPose = FFVPoseSnapshot();
	for (int32 Track = 0; Track < Tracks.Num(); ++Track)
	{
		OutPose.Bones.Add(Tracks[Track], Transforms[Track]);
	}
	for (int32 Curve = 0; Curve < Curves.Num(); ++Curve)
	{
		OutPose.Curves.Add(Curves[Curve], Values[Curve]);
	}
	return true;
}

FFVPoseSnapshot FFVPoseAuthoring::ReadBasePose(const UPoseAsset& Asset)
{
	TArray<FTransform> Transforms;
	TArray<float> Values;
	Asset.GetBasePoseTransform(Transforms, Values);

	FFVPoseSnapshot Base;
	const TArray<FName>& Tracks = Asset.GetTrackNames();
	for (int32 Track = 0; Track < Tracks.Num() && Track < Transforms.Num(); ++Track)
	{
		Base.Bones.Add(Tracks[Track], Transforms[Track]);
	}
	const TArray<FName> Curves = Asset.GetCurveFNames();
	for (int32 Curve = 0; Curve < Curves.Num(); ++Curve)
	{
		Base.Curves.Add(Curves[Curve], Values.IsValidIndex(Curve) ? Values[Curve] : 0.f);
	}
	return Base;
}

TArray<FName> FFVPoseAuthoring::GetChangedBones(const UPoseAsset& Asset, FName Pose)
{
	TArray<FName> Changed;
	FFVPoseSnapshot Snapshot;
	if (!ReadPose(Asset, Pose, Snapshot))
	{
		return Changed;
	}

	const UFVAnimationSettings& Settings = UFVAnimationSettings::Get();
	const FFVPoseSnapshot Base = ReadBasePose(Asset);
	for (const TPair<FName, FTransform>& Bone : Snapshot.Bones)
	{
		const FTransform* BaseTransform = Base.Bones.Find(Bone.Key);
		if (BaseTransform && IsChanged(Bone.Value, *BaseTransform, Settings))
		{
			Changed.Add(Bone.Key);
		}
	}
	return Changed;
}

USkeletalMesh* FFVPoseAuthoring::FindMesh(const UPoseAsset& Asset)
{
	if (USkeletalMesh* Mesh = Asset.GetPreviewMesh())
	{
		return Mesh;
	}
	const USkeleton* Skeleton = Asset.GetSkeleton();
	return Skeleton ? Skeleton->GetPreviewMesh() : nullptr;
}

bool FFVPoseAuthoring::CreatePose(UPoseAsset& Asset, FName Pose, const USkeletalMeshComponent& Source, FText& OutError)
{
	if (!ValidateNewName(Asset, Pose, OutError) || !ValidateSource(Asset, Source, OutError))
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("CreatePose", "Create Pose"));
	Asset.Modify();
	Asset.AddOrUpdatePose(Pose, &Source, true);
	return true;
}

bool FFVPoseAuthoring::UpdatePose(UPoseAsset& Asset, FName Pose, const USkeletalMeshComponent& Source, FText& OutError)
{
	if (!ValidateExisting(Asset, Pose, OutError) || !ValidateSource(Asset, Source, OutError))
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("UpdatePose", "Update Pose"));
	Asset.Modify();
	Asset.AddOrUpdatePose(Pose, &Source, true);
	return true;
}

bool FFVPoseAuthoring::RenamePose(UPoseAsset& Asset, FName Pose, FName NewName, FText& OutError)
{
	if (!ValidateExisting(Asset, Pose, OutError) || !ValidateNewName(Asset, NewName, OutError))
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("RenamePose", "Rename Pose"));
	Asset.Modify();
	return Asset.ModifyPoseName(Pose, NewName);
}

bool FFVPoseAuthoring::DeletePoses(UPoseAsset& Asset, const TArray<FName>& Poses, FText& OutError)
{
	if (Poses.IsEmpty())
	{
		OutError = LOCTEXT("ErrNoSelection", "No poses selected.");
		return false;
	}
	for (const FName Pose : Poses)
	{
		if (!ValidateExisting(Asset, Pose, OutError))
		{
			return false;
		}
	}

	const FScopedTransaction Transaction(LOCTEXT("DeletePoses", "Delete Poses"));
	Asset.Modify();
	Asset.DeletePoses(TSet<FName>(Poses).Array());
	return true;
}

bool FFVPoseAuthoring::DuplicatePose(UPoseAsset& Asset, FName Pose, FName NewName, FText& OutError)
{
	FFVPoseSnapshot Snapshot;
	if (!ValidateExisting(Asset, Pose, OutError) || !ValidateNewName(Asset, NewName, OutError))
	{
		return false;
	}
	if (!ReadPose(Asset, Pose, Snapshot))
	{
		OutError = LOCTEXT("ErrRead", "The pose data could not be read.");
		return false;
	}

	FScopedTransaction Transaction(LOCTEXT("DuplicatePose", "Duplicate Pose"));
	Asset.Modify();
	if (!WritePose(Asset, NewName, Snapshot, OutError))
	{
		Transaction.Cancel();
		return false;
	}
	return true;
}

bool FFVPoseAuthoring::WritePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError)
{
	USkeletalMesh* Mesh = FindMesh(Asset);
	if (!Mesh || Mesh->GetSkeleton() != Asset.GetSkeleton())
	{
		OutError = LOCTEXT("ErrNoMesh", "Set a preview mesh on the Pose Asset or its skeleton.");
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
	const TArray<FTransform>& RefPose = RefSkeleton.GetRefBonePose();
	TArray<FTransform> ComponentSpace;
	ComponentSpace.SetNum(RefSkeleton.GetNum());
	for (int32 Bone = 0; Bone < RefSkeleton.GetNum(); ++Bone)
	{
		const FTransform* Local = Snapshot.Bones.Find(RefSkeleton.GetBoneName(Bone));
		const int32 Parent = RefSkeleton.GetParentIndex(Bone);
		const FTransform& BoneLocal = Local ? *Local : RefPose[Bone];
		ComponentSpace[Bone] = Parent == INDEX_NONE ? BoneLocal : BoneLocal * ComponentSpace[Parent];
	}

	UFVPoseWriterComponent* Writer = NewObject<UFVPoseWriterComponent>(GetTransientPackage());
	if (!Writer->SetPose(*Mesh, ComponentSpace, Snapshot.Curves))
	{
		OutError = LOCTEXT("ErrWriter", "The pose could not be prepared on the preview mesh.");
		return false;
	}
	Asset.AddOrUpdatePose(Pose, Writer, true);
	return true;
}

#undef LOCTEXT_NAMESPACE
