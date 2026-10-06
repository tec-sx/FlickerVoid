#include "FVNavigationStatics.h"

#include "FVMapDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigationStatics)

FVector2D UFVNavigationStatics::WorldToMapUV(const UFVMapDefinition* Map, int32 LayerIndex, FVector Location)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	return Layer ? Layer->WorldToUV(Location) : FVector2D::ZeroVector;
}

FVector UFVNavigationStatics::MapUVToWorld(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D UV, float Z)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	return Layer ? Layer->UVToWorld(UV, Z) : FVector::ZeroVector;
}
