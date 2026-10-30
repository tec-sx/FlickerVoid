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
	const bool bLocal = Space == ERigVMTransformSpace::LocalSpace;
	FTransform Initial = Hierarchy->GetInitialGlobalTransform(Index);
	FTransform Current = Hierarchy->GetGlobalTransform(Index);
	if (bLocal)
	{
		Initial = Hierarchy->GetInitialLocalTransform(Index);
		Current = Hierarchy->GetLocalTransform(Index);
	}

	const FVector InitialScale = Initial.GetScale3D();
	if (InitialScale.GetAbsMin() < UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector Scale = Current.GetScale3D() / InitialScale;
	const FVector Size = Scale.GetAbs();
	const int32 Free = static_cast<int32>(FreeAxis);

	// The same totals as Blender's samevolume_evaluate.
	float Total = Size[Free];
	if (Mode == EFVMaintainVolumeMode::Strict)
	{
		Total = Size.X * Size.Y * Size.Z;
	}
	else if (Mode == EFVMaintainVolumeMode::Uniform)
	{
		Total = FMath::Pow(Size[Free], 3.0f);
	}
	if (Total < UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float Factor = FMath::Sqrt(FMath::Max(Volume, UE_KINDA_SMALL_NUMBER) / Total);
	FVector Kept = Scale * Factor;
	Kept[Free] = Scale[Free];

	const FVector Blended = FMath::Lerp(Scale, Kept, Alpha);
	Current.SetScale3D(Blended * InitialScale);
	if (bLocal)
	{
		Hierarchy->SetLocalTransform(Index, Current, false, bPropagateToChildren);
	}
	else
	{
		Hierarchy->SetGlobalTransform(Index, Current, false, bPropagateToChildren);
	}
}
