#include "RigUnits/FVRigUnit_MuscleBones.h"

#include "Units/RigUnitContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRigUnit_MuscleBones)

namespace FVMuscleBones
{
	int32 DominantAxis(const FVector& Axis)
	{
		const FVector Size = Axis.GetAbs();
		if (Size.X >= Size.Y && Size.X >= Size.Z)
		{
			return 0;
		}
		if (Size.Y >= Size.Z)
		{
			return 1;
		}
		return 2;
	}

	void Follow(URigHierarchy* Hierarchy, const FFVJointFollow& Joint, float Alpha)
	{
		const FRigElementKey Key(Joint.Bone, ERigElementType::Bone);
		const FRigElementKey SourceKey(Joint.Source, ERigElementType::Bone);
		const int32 Index = Hierarchy->GetIndex(Key);
		const int32 SourceIndex = Hierarchy->GetIndex(SourceKey);
		if (Index == INDEX_NONE || SourceIndex == INDEX_NONE)
		{
			return;
		}

		const FQuat Initial = Hierarchy->GetInitialGlobalTransform(Index).GetRotation();
		const FQuat SourceInitial = Hierarchy->GetInitialGlobalTransform(SourceIndex).GetRotation();
		const FQuat SourceCurrent = Hierarchy->GetGlobalTransform(SourceIndex).GetRotation();
		FTransform Current = Hierarchy->GetGlobalTransform(Index);

		const FQuat Followed = SourceCurrent * SourceInitial.Inverse() * Initial;
		const FQuat Blended = FQuat::Slerp(Current.GetRotation(), Followed, Joint.Weight * Alpha);
		Current.SetRotation(Blended.GetNormalized());
		Hierarchy->SetGlobalTransform(Index, Current, false, true);
	}

	void Stretch(URigHierarchy* Hierarchy, const FFVMuscleStretch& Muscle, float Alpha)
	{
		const FRigElementKey Key(Muscle.Bone, ERigElementType::Bone);
		const FRigElementKey TargetKey(Muscle.Target, ERigElementType::Bone);
		const int32 Index = Hierarchy->GetIndex(Key);
		const int32 TargetIndex = Hierarchy->GetIndex(TargetKey);
		if (Index == INDEX_NONE || TargetIndex == INDEX_NONE)
		{
			return;
		}

		const FTransform Initial = Hierarchy->GetInitialGlobalTransform(Index);
		const FVector TargetInitial = Hierarchy->GetInitialGlobalTransform(TargetIndex).GetLocation();
		const FVector RestLocal = Initial.InverseTransformPositionNoScale(TargetInitial);
		const float RestLength = RestLocal.Size();
		if (RestLength < UE_KINDA_SMALL_NUMBER)
		{
			return;
		}

		const FTransform Current = Hierarchy->GetGlobalTransform(Index);
		const FVector TargetCurrent = Hierarchy->GetGlobalTransform(TargetIndex).GetLocation();
		const FVector Aim = TargetCurrent - Current.GetLocation();
		const float Length = Aim.Size();
		if (Length < UE_KINDA_SMALL_NUMBER)
		{
			return;
		}

		const FVector Axis = RestLocal / RestLength;
		const FVector AxisNow = Current.GetRotation().RotateVector(Axis);
		const FQuat Turn = FQuat::FindBetweenNormals(AxisNow, Aim / Length);
		const FQuat Rotation = (Turn * Current.GetRotation()).GetNormalized();

		// Matches Blender's Stretch To with XZX volume: the aim axis scales by the stretch, the others by its
		// inverse square root raised to the volume.
		const float Ratio = Length / RestLength;
		FVector Scale(FMath::Pow(Ratio, -0.5f * Muscle.Volume));
		Scale[DominantAxis(Axis)] = Ratio;

		FTransform Driven = Current;
		Driven.SetRotation(FQuat::Slerp(Current.GetRotation(), Rotation, Alpha).GetNormalized());
		Driven.SetScale3D(FMath::Lerp(Current.GetScale3D(), Current.GetScale3D() * Scale, Alpha));
		Hierarchy->SetGlobalTransform(Index, Driven, false, true);
	}
}

FFVRigUnit_MuscleBones_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr || Alpha <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (const FFVJointFollow& Joint : Joints)
	{
		FVMuscleBones::Follow(Hierarchy, Joint, Alpha);
	}
	for (const FFVMuscleStretch& Muscle : Muscles)
	{
		FVMuscleBones::Stretch(Hierarchy, Muscle, Alpha);
	}
}
