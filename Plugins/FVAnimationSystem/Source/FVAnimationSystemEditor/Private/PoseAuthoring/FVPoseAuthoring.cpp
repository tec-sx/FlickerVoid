#include "PoseAuthoring/FVPoseAuthoring.h"

#include "Animation/AnimData/CurveIdentifier.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimSequence.h"
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

	/** Moves from Base towards Target by Alpha; Alpha above 1 exaggerates. */
	FTransform BlendFromBase(const FTransform& Base, const FTransform& Target, float Alpha)
	{
		FQuat Delta = Base.GetRotation().Inverse() * Target.GetRotation();
		Delta.EnforceShortestArcWith(FQuat::Identity);
		FVector Axis;
		double Angle;
		Delta.ToAxisAndAngle(Axis, Angle);
		const FQuat Rotation = Base.GetRotation() * FQuat(Axis, Angle * Alpha);
		return FTransform(Rotation.GetNormalized(),
			FMath::Lerp(Base.GetTranslation(), Target.GetTranslation(), double(Alpha)),
			FMath::Lerp(Base.GetScale3D(), Target.GetScale3D(), double(Alpha)));
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

bool FFVPoseAuthoring::CreatePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError)
{
	if (!ValidateNewName(Asset, Pose, OutError))
	{
		return false;
	}

	FScopedTransaction Transaction(LOCTEXT("CreatePose", "Create Pose"));
	Asset.Modify();
	if (!WritePose(Asset, Pose, Snapshot, OutError))
	{
		Transaction.Cancel();
		return false;
	}
	return true;
}

bool FFVPoseAuthoring::UpdatePose(UPoseAsset& Asset, FName Pose, const FFVPoseSnapshot& Snapshot, FText& OutError)
{
	if (!ValidateExisting(Asset, Pose, OutError))
	{
		return false;
	}

	FScopedTransaction Transaction(LOCTEXT("UpdatePose", "Update Pose"));
	Asset.Modify();
	if (!WritePose(Asset, Pose, Snapshot, OutError))
	{
		Transaction.Cancel();
		return false;
	}
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

FName FFVPoseAuthoring::MakeUniquePoseName(const UPoseAsset& Asset, const FString& Base)
{
	FName Name(*Base);
	for (int32 Suffix = 1; Asset.ContainsPose(Name); ++Suffix)
	{
		Name = FName(*FString::Printf(TEXT("%s_%d"), *Base, Suffix));
	}
	return Name;
}

bool FFVPoseAuthoring::CreateInbetweens(UPoseAsset& Asset, FName Pose, int32 Count, TArray<FName>& OutPoses, FText& OutError)
{
	OutPoses.Reset();
	FFVPoseSnapshot Target;
	if (!ValidateExisting(Asset, Pose, OutError) || !ReadPose(Asset, Pose, Target))
	{
		return false;
	}
	if (Count < 1 || Count > 9)
	{
		OutError = LOCTEXT("ErrInbetweenCount", "Make between 1 and 9 inbetweens.");
		return false;
	}

	TArray<float> Alphas;
	for (int32 Index = 1; Index <= Count; ++Index)
	{
		const float Alpha = float(Index) / float(Count + 1);
		const FName Name(*FString::Printf(TEXT("%s_ib%d"), *Pose.ToString(), FMath::RoundToInt(Alpha * 100.f)));
		if (!ValidateNewName(Asset, Name, OutError) || OutPoses.Contains(Name))
		{
			return false;
		}
		OutPoses.Add(Name);
		Alphas.Add(Alpha);
	}

	const FFVPoseSnapshot Base = ReadBasePose(Asset);
	FScopedTransaction Transaction(LOCTEXT("CreateInbetweens", "Create Inbetweens"));
	Asset.Modify();
	for (int32 Index = 0; Index < OutPoses.Num(); ++Index)
	{
		FFVPoseSnapshot Inbetween = Target;
		for (TPair<FName, FTransform>& Bone : Inbetween.Bones)
		{
			if (const FTransform* BaseTransform = Base.Bones.Find(Bone.Key))
			{
				Bone.Value = BlendFromBase(*BaseTransform, Bone.Value, Alphas[Index]);
			}
		}
		for (TPair<FName, float>& Curve : Inbetween.Curves)
		{
			Curve.Value = FMath::Lerp(Base.Curves.FindRef(Curve.Key), Curve.Value, Alphas[Index]);
		}
		if (!WritePose(Asset, OutPoses[Index], Inbetween, OutError))
		{
			Transaction.Cancel();
			OutPoses.Reset();
			return false;
		}
	}
	return true;
}

bool FFVPoseAuthoring::PastePose(UPoseAsset& Asset, const TArray<FName>& Targets, const FFVPoseSnapshot& Clipboard, const TSet<FName>& Bones, float Multiplier, FText& OutError)
{
	if (Targets.IsEmpty())
	{
		OutError = LOCTEXT("ErrNoSelection", "No poses selected.");
		return false;
	}
	if (!FMath::IsFinite(Multiplier))
	{
		OutError = LOCTEXT("ErrMultiplier", "The multiplier must be a number.");
		return false;
	}

	TArray<FFVPoseSnapshot> Pasted;
	const FFVPoseSnapshot Base = ReadBasePose(Asset);
	for (const FName Target : Targets)
	{
		FFVPoseSnapshot& Snapshot = Pasted.AddDefaulted_GetRef();
		if (!ValidateExisting(Asset, Target, OutError) || !ReadPose(Asset, Target, Snapshot))
		{
			return false;
		}
		for (const TPair<FName, FTransform>& Bone : Clipboard.Bones)
		{
			if (Bones.IsEmpty() || Bones.Contains(Bone.Key))
			{
				const FTransform* BaseTransform = Base.Bones.Find(Bone.Key);
				Snapshot.Bones.Add(Bone.Key, BlendFromBase(BaseTransform ? *BaseTransform : Bone.Value, Bone.Value, Multiplier));
			}
		}
		if (Bones.IsEmpty())
		{
			for (TPair<FName, float>& Curve : Snapshot.Curves)
			{
				if (const float* Value = Clipboard.Curves.Find(Curve.Key))
				{
					const float BaseValue = Base.Curves.FindRef(Curve.Key);
					Curve.Value = BaseValue + (*Value - BaseValue) * Multiplier;
				}
			}
		}
	}

	FScopedTransaction Transaction(LOCTEXT("PastePose", "Paste Pose"));
	Asset.Modify();
	for (int32 Index = 0; Index < Targets.Num(); ++Index)
	{
		if (!WritePose(Asset, Targets[Index], Pasted[Index], OutError))
		{
			Transaction.Cancel();
			return false;
		}
	}
	return true;
}

bool FFVPoseAuthoring::SetCurveValue(UPoseAsset& Asset, FName Pose, FName Curve, float Value, FText& OutError)
{
	FFVPoseSnapshot Snapshot;
	if (!ValidateExisting(Asset, Pose, OutError) || !ReadPose(Asset, Pose, Snapshot))
	{
		return false;
	}
	float* Current = Snapshot.Curves.Find(Curve);
	if (!Current)
	{
		OutError = FText::Format(LOCTEXT("ErrNoCurve", "The Pose Asset has no curve named {0}."), FText::FromName(Curve));
		return false;
	}

	*Current = Value;
	FScopedTransaction Transaction(LOCTEXT("SetCurveValue", "Set Pose Curve"));
	Asset.Modify();
	if (!WritePose(Asset, Pose, Snapshot, OutError))
	{
		Transaction.Cancel();
		return false;
	}
	return true;
}

TArray<FName> FFVPoseAuthoring::GetAddableCurves(const UPoseAsset& Asset)
{
	TArray<FName> Curves;
	if (const USkeleton* Skeleton = Asset.GetSkeleton())
	{
		Skeleton->GetCurveMetaDataNames(Curves);
	}
	const TArray<FName> Existing = Asset.GetCurveFNames();
	Curves.RemoveAll([&Existing](FName Curve) { return Existing.Contains(Curve); });
	Curves.Sort(FNameLexicalLess());
	return Curves;
}

bool FFVPoseAuthoring::AddCurve(UPoseAsset& Asset, FName Curve, FText& OutError)
{
	USkeleton* Skeleton = Asset.GetSkeleton();
	if (!Skeleton || !GetAddableCurves(Asset).Contains(Curve))
	{
		OutError = FText::Format(LOCTEXT("ErrCurveNotAddable", "{0} isn't a skeleton curve the Pose Asset can add."), FText::FromName(Curve));
		return false;
	}

	const TArray<FName> PoseNames = Asset.GetPoseFNames();
	if (PoseNames.IsEmpty())
	{
		OutError = LOCTEXT("ErrNoPosesForCurve", "Add a pose before adding curves.");
		return false;
	}
	TArray<FFVPoseSnapshot> Poses;
	for (const FName Pose : PoseNames)
	{
		if (!ReadPose(Asset, Pose, Poses.AddDefaulted_GetRef()))
		{
			OutError = LOCTEXT("ErrRead", "The pose data could not be read.");
			return false;
		}
	}

	// Pose Assets only take new curves from a source animation: one key per pose, rebuilt with the existing names.
	const int32 NumKeys = FMath::Max(Poses.Num(), 2);
	const FFrameRate FrameRate(30, 1);
	UAnimSequence* Sequence = NewObject<UAnimSequence>(GetTransientPackage(), NAME_None, RF_Transient);
	Sequence->SetSkeleton(Skeleton);
	IAnimationDataController& Controller = Sequence->GetController();
	Controller.InitializeModel();
	Controller.OpenBracket(LOCTEXT("BuildCurveSource", "Build curve source"), false);
	Controller.SetFrameRate(FrameRate, false);
	Controller.SetNumberOfFrames(FFrameNumber(NumKeys - 1), false);
	for (const FName Track : Asset.GetTrackNames())
	{
		TArray<FVector> Positions;
		TArray<FQuat> Rotations;
		TArray<FVector> Scales;
		for (int32 Key = 0; Key < NumKeys; ++Key)
		{
			const FTransform& Transform = Poses[FMath::Min(Key, Poses.Num() - 1)].Bones.FindChecked(Track);
			Positions.Add(Transform.GetTranslation());
			Rotations.Add(Transform.GetRotation());
			Scales.Add(Transform.GetScale3D());
		}
		Controller.AddBoneCurve(Track, false);
		Controller.SetBoneTrackKeys(Track, Positions, Rotations, Scales, false);
	}
	TArray<FName> CurveNames = Asset.GetCurveFNames();
	CurveNames.Add(Curve);
	for (const FName CurveName : CurveNames)
	{
		TArray<FRichCurveKey> Keys;
		for (int32 Key = 0; Key < NumKeys; ++Key)
		{
			const float Value = Poses[FMath::Min(Key, Poses.Num() - 1)].Curves.FindRef(CurveName);
			Keys.Add(FRichCurveKey(float(FrameRate.AsSeconds(FFrameNumber(Key))), Value));
		}
		const FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
		Controller.AddCurve(CurveId, AACF_DefaultCurve, false);
		Controller.SetCurveKeys(CurveId, Keys, false);
	}
	Controller.NotifyPopulated();
	Controller.CloseBracket(false);

	FScopedTransaction Transaction(LOCTEXT("AddCurve", "Add Pose Curve"));
	Asset.Modify();
	const TObjectPtr<UAnimSequence> SourceAnimation = Asset.SourceAnimation;
	const FGuid SourceGuid = Asset.SourceAnimationRawDataGUID;
	Asset.UpdatePoseFromAnimation(Sequence);
	Asset.SourceAnimation = SourceAnimation;
	Asset.SourceAnimationRawDataGUID = SourceGuid;

	// A single pose is keyed twice, which adds one spare pose.
	TArray<FName> Spare = Asset.GetPoseFNames();
	Spare.RemoveAll([&PoseNames](FName Pose) { return PoseNames.Contains(Pose); });
	if (!Spare.IsEmpty())
	{
		Asset.DeletePoses(Spare);
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
