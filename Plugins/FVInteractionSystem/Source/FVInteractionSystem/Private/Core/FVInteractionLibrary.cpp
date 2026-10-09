#include "Core/FVInteractionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionLibrary)

UMeshComponent* UFVInteractionLibrary::FindMeshByTag(const FName Tag, const AActor* Actor)
{
	if (!IsValid(Actor)) 
		return nullptr;

	TArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents(MeshComponents);

	for (UMeshComponent* const& MeshComponent : MeshComponents)
	{
		if (MeshComponent && MeshComponent->ComponentHasTag(Tag))
		{
			return MeshComponent;
		}
	}

	return nullptr;
}
