#include "PoseAuthoring/FVPoseWriterComponent.h"

#include "Engine/SkeletalMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPoseWriterComponent)

bool UFVPoseWriterComponent::SetPose(USkeletalMesh& Mesh, const TArray<FTransform>& ComponentSpace, const TMap<FName, float>& Curves)
{
	SetSkeletalMeshAsset(&Mesh);
	if (!AllocateTransformData() || GetNumComponentSpaceTransforms() != ComponentSpace.Num())
	{
		return false;
	}

	GetEditableComponentSpaceTransforms() = ComponentSpace;
	FBlendedHeapCurve& EditableCurves = GetEditableAnimationCurves();
	EditableCurves.Empty();
	for (const TPair<FName, float>& Curve : Curves)
	{
		EditableCurves.Set(Curve.Key, Curve.Value);
	}

	// Never ticked, so make the reads used by AddOrUpdatePose see the buffers written above.
	CurrentReadComponentTransforms = CurrentEditableComponentTransforms;
	return true;
}
