#include "RigUnits/FVRigUnit_MaintainVolume.h"

#include "Units/RigUnitContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRigUnit_MaintainVolume)

FFVRigUnit_MaintainVolume_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr || Alpha <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}
	if (!CachedItem.UpdateCache(Item, Hierarchy))
	{
		UE_CONTROLRIG_RIGUNIT_REPORT_WARNING(TEXT("Item '%s' is not valid."), *Item.ToString());
		return;
	}

	const int32 Index = CachedItem.GetIndex();
	const FVector InitialScale = Hierarchy->GetInitialLocalTransform(Index).GetScale3D();
	if (InitialScale.GetAbsMin() < UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	FTransform Current = Hierarchy->GetLocalTransform(Index);
	const FVector Scale = Current.GetScale3D() / InitialScale;
	const int32 Free = static_cast<int32>(FreeAxis);

	float Total = FMath::Abs(Scale[Free]);
	if (Mode == EFVMaintainVolumeMode::Uniform)
	{
		Total = FMath::Abs(Scale.X * Scale.Y * Scale.Z);
	}
	if (Total < UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Same as Blender: Strict sets the other axes to the factor, the other modes multiply them by it.
	const float Factor = FMath::Sqrt(FMath::Max(Volume, UE_KINDA_SMALL_NUMBER) / Total);
	FVector Kept = Scale;
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		if (Axis == Free)
		{
			continue;
		}
		if (Mode == EFVMaintainVolumeMode::Strict)
		{
			Kept[Axis] = Scale[Axis] < 0.0f ? -Factor : Factor;
		}
		else
		{
			Kept[Axis] = Scale[Axis] * Factor;
		}
	}

	const FVector Blended = FMath::Lerp(Scale, Kept, Alpha);
	Current.SetScale3D(Blended * InitialScale);
	Hierarchy->SetLocalTransform(Index, Current, false, bPropagateToChildren);
}
