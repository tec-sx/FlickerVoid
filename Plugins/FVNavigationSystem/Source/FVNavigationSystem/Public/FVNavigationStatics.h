#pragma once

#include "CoreMinimal.h"
#include "FVNavigationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVNavigationStatics.generated.h"

class UFVMapDefinition;

/** Map projection helpers. Markers, waypoints and maps live on UFVNavigationSubsystem. */
UCLASS()
class FVNAVIGATIONSYSTEM_API UFVNavigationStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Texture UV of a world location on a map layer; (0, 0) is the north-west corner. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	static FVector2D WorldToMapUV(const UFVMapDefinition* Map, int32 LayerIndex, FVector Location);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	static FVector MapUVToWorld(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D UV, float Z = 0.f);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	static bool IsValidMarker(FFVMarkerHandle Marker) { return Marker.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (DisplayName = "Equal (Marker Handle)", CompactNodeTitle = "=="))
	static bool EqualMarker(FFVMarkerHandle A, FFVMarkerHandle B) { return A == B; }
};
