#pragma once

#include "CoreMinimal.h"
#include "FVNavigationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVNavigationLibrary.generated.h"

/**
 * Blueprint helpers for navigation UI that the map view, compass and marker widgets don't cover.
 * Markers, waypoints and maps live on UFVNavigationSubsystem.
 */
UCLASS()
class FVNAVIGATIONSYSTEM_API UFVNavigationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker",
		meta = (ToolTip = "Text such as 85 m or 1.2 km from a distance in centimetres."))
	static FText FormatDistance(float Distance);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker")
	static bool IsValidMarker(FFVMarkerHandle Marker) { return Marker.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker", meta = (DisplayName = "Equal (Marker Handle)", CompactNodeTitle = "=="))
	static bool EqualMarker(FFVMarkerHandle A, FFVMarkerHandle B) { return A == B; }
};
