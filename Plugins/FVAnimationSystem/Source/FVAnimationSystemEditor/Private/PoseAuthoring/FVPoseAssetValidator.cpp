#include "PoseAuthoring/FVPoseAssetValidator.h"

#include "Animation/PoseAsset.h"
#include "FVAnimationSettings.h"
#include "FVMirrorNameRule.h"
#include "PoseAuthoring/FVPoseAuthoring.h"
#include "PoseAuthoring/FVPoseMirror.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPoseAssetValidator)

#define LOCTEXT_NAMESPACE "FVPoseAssetValidator"

namespace
{
	bool IsSameTransform(const FTransform& A, const FTransform& B, const UFVAnimationSettings& Settings)
	{
		return FMath::RadiansToDegrees(A.GetRotation().AngularDistance(B.GetRotation())) <= FMath::Max(Settings.ChangedRotationTolerance, 0.05f)
			&& FVector::Dist(A.GetTranslation(), B.GetTranslation()) <= FMath::Max(Settings.ChangedTranslationTolerance, 0.05f);
	}
}

bool UFVPoseAssetValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return Cast<UPoseAsset>(InObject) != nullptr;
}

EDataValidationResult UFVPoseAssetValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UPoseAsset* Asset = CastChecked<UPoseAsset>(InAsset);
	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	const UFVAnimationSettings& Settings = UFVAnimationSettings::Get();
	bool bWarned = false;

	for (const FName Pose : Asset->GetPoseFNames())
	{
		const EFVBodySide Side = Rule.GetNameSide(Pose.ToString());
		if (Side == EFVBodySide::Centre)
		{
			continue;
		}

		bool bOwnSide = false;
		bool bOtherSide = false;
		for (const FName Bone : FFVPoseAuthoring::GetChangedBones(*Asset, Pose))
		{
			const EFVBodySide BoneSide = Rule.GetBoneSide(Bone.ToString());
			bOwnSide |= BoneSide == Side;
			bOtherSide |= BoneSide != Side && BoneSide != EFVBodySide::Centre;
		}
		if (bOtherSide && !bOwnSide)
		{
			AssetWarning(Asset, FText::Format(LOCTEXT("SideMismatch", "Pose {0} is named for one side but only moves bones of the other side."), FText::FromName(Pose)));
			bWarned = true;
			continue;
		}

		FFVPoseSnapshot Existing;
		const FName Counterpart = FFVPoseMirror::GetMirrorPoseName(Pose);
		if (Side != EFVBodySide::Left || !FFVPoseAuthoring::ReadPose(*Asset, Counterpart, Existing))
		{
			continue;
		}
		FName Target;
		FFVPoseSnapshot Expected;
		FText Error;
		if (!FFVPoseMirror::BuildMirroredPose(*Asset, Pose, EFVMirrorAxis::Default, Target, Expected, Error))
		{
			continue;
		}
		for (const TPair<FName, FTransform>& Bone : Expected.Bones)
		{
			const FTransform* Current = Existing.Bones.Find(Bone.Key);
			if (Rule.GetBoneSide(Bone.Key.ToString()) != EFVBodySide::Centre && Rule.GetBoneSide(Bone.Key.ToString()) != Side
				&& Current && !IsSameTransform(*Current, Bone.Value, Settings))
			{
				AssetWarning(Asset, FText::Format(LOCTEXT("CounterpartDiffers", "Pose {0} no longer mirrors {1} (bone {2}). Mirror {1} again, or rename one of them."),
					FText::FromName(Counterpart), FText::FromName(Pose), FText::FromName(Bone.Key)));
				bWarned = true;
				break;
			}
		}
	}

	if (!bWarned)
	{
		AssetPasses(Asset);
	}
	return EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE
