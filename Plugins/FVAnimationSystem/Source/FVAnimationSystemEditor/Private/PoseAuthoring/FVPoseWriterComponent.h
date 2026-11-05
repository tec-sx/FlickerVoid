#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "FVPoseWriterComponent.generated.h"

/**
 * Offscreen, never registered mesh component that holds a computed pose, so it can be written with
 * UPoseAsset::AddOrUpdatePose, the same public path as Persona's Insert Pose.
 */
UCLASS(Transient, NotBlueprintable, HideDropdown)
class UFVPoseWriterComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** ComponentSpace holds one transform per bone of Mesh's reference skeleton. */
	bool SetPose(USkeletalMesh& Mesh, const TArray<FTransform>& ComponentSpace, const TMap<FName, float>& Curves);
};
