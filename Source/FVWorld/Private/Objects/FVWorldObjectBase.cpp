#include "Objects/FVWorldObjectBase.h"
#include "Components/StaticMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVWorldObjectBase)

AFVWorldObjectBase::AFVWorldObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
}

UStaticMeshComponent* AFVWorldObjectBase::GetMeshComponent() const
{
	return MeshComponent;
}
