#include "AnimationRuntime.h"
#include "Animation/PoseAsset.h"
#include "Engine/SkeletalMesh.h"
#include "FVMirrorNameRule.h"
#include "Misc/AutomationTest.h"
#include "PoseAuthoring/FVPoseAuthoring.h"
#include "PoseAuthoring/FVPoseMirror.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FVPoseAuthoringTests
{
	/** Mannequins are loaded read-only; every Pose Asset in these tests is transient. */
	USkeletalMesh* LoadMannequin()
	{
		for (const TCHAR* Path : {
			TEXT("/Game/Characters/UE5_Mannequins/Meshes/SKM_Manny.SKM_Manny"),
			TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"),
			TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP") })
		{
			if (USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, Path))
			{
				return Mesh;
			}
		}
		return nullptr;
	}

	UPoseAsset* MakePoseAsset(USkeletalMesh& Mesh)
	{
		UPoseAsset* Asset = NewObject<UPoseAsset>(GetTransientPackage(), NAME_None, RF_Transient);
		Asset->SetSkeleton(Mesh.GetSkeleton());
		Asset->SetPreviewMesh(&Mesh, false);
		return Asset;
	}

	FFVPoseSnapshot RefPose(const USkeletalMesh& Mesh)
	{
		FFVPoseSnapshot Snapshot;
		const FReferenceSkeleton& RefSkeleton = Mesh.GetRefSkeleton();
		for (int32 Bone = 0; Bone < RefSkeleton.GetNum(); ++Bone)
		{
			Snapshot.Bones.Add(RefSkeleton.GetBoneName(Bone), RefSkeleton.GetRefBonePose()[Bone]);
		}
		return Snapshot;
	}

	void Rotate(FFVPoseSnapshot& Snapshot, FName Bone, const FQuat& Delta)
	{
		FTransform& Local = Snapshot.Bones.FindChecked(Bone);
		Local.SetRotation(Local.GetRotation() * Delta);
	}

	FVector ComponentLocation(const USkeletalMesh& Mesh, const FFVPoseSnapshot& Snapshot, FName Bone)
	{
		const FReferenceSkeleton& RefSkeleton = Mesh.GetRefSkeleton();
		FTransform Result = FTransform::Identity;
		for (int32 Index = RefSkeleton.FindBoneIndex(Bone); Index != INDEX_NONE; Index = RefSkeleton.GetParentIndex(Index))
		{
			Result = Result * Snapshot.Bones.FindChecked(RefSkeleton.GetBoneName(Index));
		}
		return Result.GetLocation();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVMirrorNameRuleTest, "FlickerVoid.Animation.MirrorNames",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVMirrorNameRuleTest::RunTest(const FString& Parameters)
{
	const FFVMirrorNameRule Rule({ FFVSidePair() });

	TestEqual(TEXT("Left bone"), Rule.MirrorBoneName(TEXT("thigh_twist_01_l")), FString(TEXT("thigh_twist_01_r")));
	TestEqual(TEXT("Right bone"), Rule.MirrorBoneName(TEXT("hand_r")), FString(TEXT("hand_l")));
	TestEqual(TEXT("Upper case kept"), Rule.MirrorBoneName(TEXT("Hand_L")), FString(TEXT("Hand_R")));
	TestTrue(TEXT("Centre bone has no mirror"), Rule.MirrorBoneName(TEXT("spine_03")).IsEmpty());
	TestTrue(TEXT("Centre bone side"), Rule.GetBoneSide(TEXT("head")) == EFVBodySide::Centre);
	TestTrue(TEXT("Suffix only"), Rule.GetBoneSide(TEXT("upperarm_l_twist")) == EFVBodySide::Centre);

	TestEqual(TEXT("Pose token"), Rule.MirrorName(TEXT("upperarm_l_up_90")), FString(TEXT("upperarm_r_up_90")));
	TestEqual(TEXT("Curve token"), Rule.MirrorName(TEXT("CR_upperarm_r_up")), FString(TEXT("CR_upperarm_l_up")));
	TestTrue(TEXT("Pose side"), Rule.GetNameSide(TEXT("upperarm_l_up_90")) == EFVBodySide::Left);
	TestTrue(TEXT("Not a token"), Rule.GetNameSide(TEXT("upperarm_lower")) == EFVBodySide::Centre);
	TestTrue(TEXT("No token, no mirror"), Rule.MirrorName(TEXT("ArmRaise")).IsEmpty());
	TestTrue(TEXT("Both sides is centre"), Rule.GetNameSide(TEXT("hands_l_r")) == EFVBodySide::Centre);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVPoseMirrorTest, "FlickerVoid.Animation.MeatbagFactory.Mirror",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVPoseMirrorTest::RunTest(const FString& Parameters)
{
	using namespace FVPoseAuthoringTests;

	USkeletalMesh* Mesh = LoadMannequin();
	if (!TestNotNull(TEXT("Mannequin mesh"), Mesh))
	{
		return false;
	}
	UPoseAsset* Asset = MakePoseAsset(*Mesh);
	const FName Left("upperarm_l_up");
	const FName Right("upperarm_r_up");
	FText Error;

	FFVPoseSnapshot Source = RefPose(*Mesh);
	Rotate(Source, "upperarm_l", FQuat(FVector(0.3, 0.2, 1.0).GetSafeNormal(), FMath::DegreesToRadians(70.0)));
	Rotate(Source, "lowerarm_l", FQuat(FVector::ZAxisVector, FMath::DegreesToRadians(40.0)));
	TestTrue(TEXT("Write source"), FFVPoseAuthoring::WritePose(*Asset, Left, Source, Error));

	const TArray<FName> Changed = FFVPoseAuthoring::GetChangedBones(*Asset, Left);
	TestTrue(TEXT("upperarm_l changed"), Changed.Contains(FName("upperarm_l")));
	TestFalse(TEXT("hand_l unchanged"), Changed.Contains(FName("hand_l")));

	TArray<FName> Targets;
	TestTrue(TEXT("Mirror creates counterpart"), FFVPoseMirror::MirrorPoses(*Asset, { Left }, EFVMirrorAxis::Default, Targets, Error));
	TestTrue(TEXT("Counterpart name"), Targets.Num() == 1 && Targets[0] == Right);

	FFVPoseSnapshot Mirrored;
	TestTrue(TEXT("Counterpart exists"), FFVPoseAuthoring::ReadPose(*Asset, Right, Mirrored));
	const TArray<FName> MirroredChanged = FFVPoseAuthoring::GetChangedBones(*Asset, Right);
	TestTrue(TEXT("upperarm_r changed"), MirroredChanged.Contains(FName("upperarm_r")));
	TestFalse(TEXT("Source side untouched"), MirroredChanged.Contains(FName("upperarm_l")));

	const FVector SourceHand = FAnimationRuntime::MirrorVector(ComponentLocation(*Mesh, Source, "hand_l"), EAxis::X);
	const FVector MirroredHand = ComponentLocation(*Mesh, Mirrored, "hand_r");
	TestTrue(*FString::Printf(TEXT("hand_r reflects hand_l (%s vs %s)"), *SourceHand.ToString(), *MirroredHand.ToString()),
		SourceHand.Equals(MirroredHand, 0.5));

	TestFalse(TEXT("Pose and counterpart together are refused"), FFVPoseMirror::MirrorPoses(*Asset, { Left, Right }, EFVMirrorAxis::Default, Targets, Error));

	TestTrue(TEXT("Mirror back updates the original"), FFVPoseMirror::MirrorPoses(*Asset, { Right }, EFVMirrorAxis::Default, Targets, Error));
	FFVPoseSnapshot RoundTrip;
	FFVPoseAuthoring::ReadPose(*Asset, Left, RoundTrip);
	for (const FName Bone : { FName("upperarm_l"), FName("lowerarm_l") })
	{
		const FQuat Expected = Source.Bones.FindChecked(Bone).GetRotation();
		const FQuat Actual = RoundTrip.Bones.FindChecked(Bone).GetRotation();
		TestTrue(*FString::Printf(TEXT("%s round trip"), *Bone.ToString()), FMath::RadiansToDegrees(Expected.AngularDistance(Actual)) < 0.05);
	}
	TestEqual(TEXT("Update keeps the pose count"), Asset->GetNumPoses(), 2);

	FFVPoseSnapshot Twist = Source;
	Rotate(Twist, "spine_03", FQuat(FVector::XAxisVector, FMath::DegreesToRadians(15.0)));
	FFVPoseAuthoring::WritePose(*Asset, "torso_l_twist", Twist, Error);
	TestTrue(TEXT("Mirror a pose with a centre bone"), FFVPoseMirror::MirrorPoses(*Asset, { "torso_l_twist" }, EFVMirrorAxis::Default, Targets, Error));
	TestFalse(TEXT("Centre bones are not mirrored"), FFVPoseAuthoring::GetChangedBones(*Asset, "torso_r_twist").Contains(FName("spine_03")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVPoseMirrorRefusalTest, "FlickerVoid.Animation.MeatbagFactory.MirrorRefusals",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVPoseMirrorRefusalTest::RunTest(const FString& Parameters)
{
	using namespace FVPoseAuthoringTests;

	USkeletalMesh* Mesh = LoadMannequin();
	if (!TestNotNull(TEXT("Mannequin mesh"), Mesh))
	{
		return false;
	}
	UPoseAsset* Asset = MakePoseAsset(*Mesh);
	FText Error;

	FFVPoseSnapshot Spine = RefPose(*Mesh);
	Rotate(Spine, "spine_03", FQuat(FVector::YAxisVector, FMath::DegreesToRadians(20.0)));
	FFVPoseAuthoring::WritePose(*Asset, "spine_bend", Spine, Error);
	FFVPoseAuthoring::WritePose(*Asset, "spine_l_lean", Spine, Error);

	TestFalse(TEXT("No side token"), FFVPoseMirror::CanMirrorPose(*Asset, "spine_bend", Error));
	TestFalse(TEXT("Centre bones only"), FFVPoseMirror::CanMirrorPose(*Asset, "spine_l_lean", Error));

	TArray<FName> Targets;
	TestFalse(TEXT("Batch is all or nothing"), FFVPoseMirror::MirrorPoses(*Asset, { "spine_l_lean" }, EFVMirrorAxis::Default, Targets, Error));
	TestEqual(TEXT("Nothing written"), Asset->GetNumPoses(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVPoseMirrorAllTest, "FlickerVoid.Animation.MeatbagFactory.MirrorAll",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVPoseMirrorAllTest::RunTest(const FString& Parameters)
{
	using namespace FVPoseAuthoringTests;

	USkeletalMesh* Mesh = LoadMannequin();
	if (!TestNotNull(TEXT("Mannequin mesh"), Mesh))
	{
		return false;
	}
	UPoseAsset* Asset = MakePoseAsset(*Mesh);
	FText Error;

	FFVPoseSnapshot Arm = RefPose(*Mesh);
	Rotate(Arm, "upperarm_l", FQuat(FVector::ZAxisVector, FMath::DegreesToRadians(60.0)));
	FFVPoseSnapshot Leg = RefPose(*Mesh);
	Rotate(Leg, "thigh_l", FQuat(FVector::YAxisVector, FMath::DegreesToRadians(50.0)));
	FFVPoseSnapshot Spine = RefPose(*Mesh);
	Rotate(Spine, "spine_03", FQuat(FVector::YAxisVector, FMath::DegreesToRadians(20.0)));
	FFVPoseSnapshot Foot = RefPose(*Mesh);
	Rotate(Foot, "foot_r", FQuat(FVector::YAxisVector, FMath::DegreesToRadians(30.0)));
	FFVPoseAuthoring::WritePose(*Asset, "upperarm_l_up", Arm, Error);
	FFVPoseAuthoring::WritePose(*Asset, "upperarm_r_up", RefPose(*Mesh), Error);
	FFVPoseAuthoring::WritePose(*Asset, "thigh_l_raise", Leg, Error);
	FFVPoseAuthoring::WritePose(*Asset, "spine_l_lean", Spine, Error);
	FFVPoseAuthoring::WritePose(*Asset, "foot_r_lift", Foot, Error);

	const FFVMirrorAllResult Plan = FFVPoseMirror::PlanMirrorAll(*Asset, EFVBodySide::Left);
	TestTrue(TEXT("Plan creates thigh_r_raise"), Plan.Created == TArray<FName>({ "thigh_r_raise" }));
	TestTrue(TEXT("Plan updates upperarm_r_up"), Plan.Updated == TArray<FName>({ "upperarm_r_up" }));
	TestTrue(TEXT("Plan skips the centre-only pose"), Plan.Skipped.Num() == 1 && Plan.Skipped[0].Key == FName("spine_l_lean"));
	TestTrue(TEXT("Plan finds the orphan"), Plan.Orphans == TArray<FName>({ "foot_r_lift" }));
	TestEqual(TEXT("Planning changes nothing"), Asset->GetNumPoses(), 5);

	FFVMirrorAllResult Kept;
	TestTrue(TEXT("Mirror All keeping orphans"), FFVPoseMirror::MirrorAll(*Asset, EFVBodySide::Left, EFVMirrorAxis::Default, false, Kept, Error));
	TestTrue(TEXT("Counterpart created"), Asset->ContainsPose("thigh_r_raise"));
	TestTrue(TEXT("Orphan kept"), Asset->ContainsPose("foot_r_lift") && !Kept.bOrphansDeleted);
	TestTrue(TEXT("upperarm_r moved"), FFVPoseAuthoring::GetChangedBones(*Asset, "upperarm_r_up").Contains(FName("upperarm_r")));

	FFVMirrorAllResult Cleaned;
	TestTrue(TEXT("Mirror All deleting orphans"), FFVPoseMirror::MirrorAll(*Asset, EFVBodySide::Left, EFVMirrorAxis::Default, true, Cleaned, Error));
	TestTrue(TEXT("Second run only updates"), Cleaned.Created.IsEmpty() && Cleaned.Updated.Num() == 2);
	TestFalse(TEXT("Orphan deleted"), Asset->ContainsPose("foot_r_lift"));
	TestEqual(TEXT("Pose count"), Asset->GetNumPoses(), 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVPoseCommandsTest, "FlickerVoid.Animation.MeatbagFactory.Commands",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVPoseCommandsTest::RunTest(const FString& Parameters)
{
	using namespace FVPoseAuthoringTests;

	USkeletalMesh* Mesh = LoadMannequin();
	if (!TestNotNull(TEXT("Mannequin mesh"), Mesh))
	{
		return false;
	}
	UPoseAsset* Asset = MakePoseAsset(*Mesh);
	FText Error;

	FFVPoseSnapshot Pose = RefPose(*Mesh);
	Rotate(Pose, "thigh_l", FQuat(FVector::YAxisVector, FMath::DegreesToRadians(45.0)));
	TestTrue(TEXT("Write"), FFVPoseAuthoring::WritePose(*Asset, "thigh_l_raise", Pose, Error));
	TestTrue(TEXT("Duplicate"), FFVPoseAuthoring::DuplicatePose(*Asset, "thigh_l_raise", "thigh_l_raise_copy", Error));
	TestFalse(TEXT("Duplicate onto an existing name"), FFVPoseAuthoring::DuplicatePose(*Asset, "thigh_l_raise", "thigh_l_raise_copy", Error));
	TestTrue(TEXT("Rename"), FFVPoseAuthoring::RenamePose(*Asset, "thigh_l_raise_copy", "thigh_l_raise_2", Error));
	TestTrue(TEXT("Renamed"), Asset->ContainsPose("thigh_l_raise_2") && !Asset->ContainsPose("thigh_l_raise_copy"));
	TestFalse(TEXT("Delete refuses unknown names"), FFVPoseAuthoring::DeletePoses(*Asset, { "thigh_l_raise", "missing" }, Error));
	TestEqual(TEXT("Nothing deleted"), Asset->GetNumPoses(), 2);
	TestTrue(TEXT("Delete"), FFVPoseAuthoring::DeletePoses(*Asset, { "thigh_l_raise", "thigh_l_raise_2" }, Error));
	TestEqual(TEXT("All deleted"), Asset->GetNumPoses(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVPoseBlendCommandsTest, "FlickerVoid.Animation.MeatbagFactory.InbetweensAndPaste",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVPoseBlendCommandsTest::RunTest(const FString& Parameters)
{
	using namespace FVPoseAuthoringTests;

	USkeletalMesh* Mesh = LoadMannequin();
	if (!TestNotNull(TEXT("Mannequin mesh"), Mesh))
	{
		return false;
	}
	UPoseAsset* Asset = MakePoseAsset(*Mesh);
	FText Error;
	const FQuat RefRotation = Mesh->GetRefSkeleton().GetRefBonePose()[Mesh->GetRefSkeleton().FindBoneIndex("thigh_l")].GetRotation();

	FFVPoseSnapshot Pose = RefPose(*Mesh);
	Rotate(Pose, "thigh_l", FQuat(FVector::YAxisVector, FMath::DegreesToRadians(80.0)));
	FFVPoseAuthoring::WritePose(*Asset, "thigh_l_raise", Pose, Error);

	TArray<FName> Inbetweens;
	TestTrue(TEXT("Inbetweens"), FFVPoseAuthoring::CreateInbetweens(*Asset, "thigh_l_raise", 3, Inbetweens, Error));
	TestTrue(TEXT("Inbetween names"), Inbetweens == TArray<FName>({ "thigh_l_raise_ib25", "thigh_l_raise_ib50", "thigh_l_raise_ib75" }));
	FFVPoseSnapshot Half;
	FFVPoseAuthoring::ReadPose(*Asset, "thigh_l_raise_ib50", Half);
	const double HalfAngle = FMath::RadiansToDegrees(RefRotation.AngularDistance(Half.Bones.FindChecked("thigh_l").GetRotation()));
	TestTrue(*FString::Printf(TEXT("Halfway is 40 degrees (%.2f)"), HalfAngle), FMath::IsNearlyEqual(HalfAngle, 40.0, 0.1));
	TestFalse(TEXT("Inbetweens refuse existing names"), FFVPoseAuthoring::CreateInbetweens(*Asset, "thigh_l_raise", 1, Inbetweens, Error));

	FFVPoseSnapshot Clipboard;
	FFVPoseAuthoring::ReadPose(*Asset, "thigh_l_raise", Clipboard);
	FFVPoseAuthoring::WritePose(*Asset, "target_l", RefPose(*Mesh), Error);
	TestTrue(TEXT("Paste at 1.5"), FFVPoseAuthoring::PastePose(*Asset, { "target_l" }, Clipboard, { "thigh_l" }, 1.5f, Error));
	FFVPoseSnapshot Pasted;
	FFVPoseAuthoring::ReadPose(*Asset, "target_l", Pasted);
	const double PastedAngle = FMath::RadiansToDegrees(RefRotation.AngularDistance(Pasted.Bones.FindChecked("thigh_l").GetRotation()));
	TestTrue(*FString::Printf(TEXT("Multiplier exaggerates to 120 degrees (%.2f)"), PastedAngle), FMath::IsNearlyEqual(PastedAngle, 120.0, 0.1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVPoseCurvesTest, "FlickerVoid.Animation.MeatbagFactory.Curves",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVPoseCurvesTest::RunTest(const FString& Parameters)
{
	using namespace FVPoseAuthoringTests;

	USkeletalMesh* Mesh = LoadMannequin();
	if (!TestNotNull(TEXT("Mannequin mesh"), Mesh))
	{
		return false;
	}
	UPoseAsset* Asset = MakePoseAsset(*Mesh);
	FText Error;

	const TArray<FName> Curves = FFVPoseAuthoring::GetAddableCurves(*Asset);
	if (Curves.IsEmpty())
	{
		AddInfo(TEXT("The mannequin skeleton has no curves; nothing to add."));
		return true;
	}

	FFVPoseSnapshot Pose = RefPose(*Mesh);
	Rotate(Pose, "hand_l", FQuat(FVector::XAxisVector, FMath::DegreesToRadians(30.0)));
	FFVPoseAuthoring::WritePose(*Asset, "hand_l_twist", Pose, Error);
	TestFalse(TEXT("Unknown curve refused"), FFVPoseAuthoring::SetCurveValue(*Asset, "hand_l_twist", Curves[0], 0.5f, Error));

	TestTrue(TEXT("Add curve"), FFVPoseAuthoring::AddCurve(*Asset, Curves[0], Error));
	TestTrue(TEXT("Curve added"), Asset->GetCurveFNames().Contains(Curves[0]));
	TestEqual(TEXT("Single pose kept, no spare"), Asset->GetNumPoses(), 1);
	TestTrue(TEXT("Pose name kept"), Asset->ContainsPose("hand_l_twist"));
	TestTrue(TEXT("Set curve value"), FFVPoseAuthoring::SetCurveValue(*Asset, "hand_l_twist", Curves[0], 0.5f, Error));

	FFVPoseSnapshot Read;
	FFVPoseAuthoring::ReadPose(*Asset, "hand_l_twist", Read);
	TestTrue(TEXT("Curve value stored"), FMath::IsNearlyEqual(Read.Curves.FindRef(Curves[0]), 0.5f));
	TestTrue(TEXT("Bones kept"), Read.Bones.FindChecked("hand_l").GetRotation().Equals(Pose.Bones.FindChecked("hand_l").GetRotation(), 1.e-4));
	return true;
}

#endif
