#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "FVMapDefinition.h"
#include "FVNavigationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVNavigationLibrary.generated.h"

class UMaterialInstanceDynamic;
class UTexture2D;

UENUM(BlueprintType)
enum class EFVMapLoadResult : uint8
{
	Loaded,
	Failed
};

/**
 * Helpers for navigation UI: map layer loading, map projection, material parameters, minimap and compass placement,
 * and world map pan, zoom and picking. Markers, waypoints and maps live on UFVNavigationSubsystem.
 *
 * World map functions share one view state: PanUV (texture UV at the centre of the view), ExtentUV (half of the
 * texture shown, in UV) and ViewSize (the widget's size in pixels).
 */
UCLASS()
class FVNAVIGATIONSYSTEM_API UFVNavigationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Maps and layers

	/** False when the map is null or has no layer at LayerIndex. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Map")
	static bool GetMapLayer(const UFVMapDefinition* Map, int32 LayerIndex, FFVMapLayer& OutLayer);

	/** Gets the layer and loads its texture in the background; Failed when the layer doesn't exist or has no texture. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation|Map",
		meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Result"))
	static void AsyncLoadMapLayer(UObject* WorldContextObject, const UFVMapDefinition* Map, int32 LayerIndex,
		FFVMapLayer& OutLayer, UTexture2D*& OutTexture, EFVMapLoadResult& Result, FLatentActionInfo LatentInfo);

	/** Texture UV of a world location on a map layer; (0, 0) is the north-west corner. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Map")
	static FVector2D WorldToMapUV(const UFVMapDefinition* Map, int32 LayerIndex, FVector Location);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Map")
	static FVector MapUVToWorld(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D UV, float Z = 0.f);

	/**
	 * Centre and half-size in texture UV of a circle of Radius world units around Center on a layer.
	 * Gives the minimap values for any layer, e.g. the outdoor layer drawn under an interior.
	 */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Map")
	static bool GetLayerViewArea(const UFVMapDefinition* Map, int32 LayerIndex, FVector Center, float Radius,
		FVector2D& OutCenterUV, FVector2D& OutExtentUV);

	// Map materials

	/** Sets a vector parameter from a 2D vector (X and Y in R and G). */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation|Material")
	static void SetMaterialVector2D(UMaterialInstanceDynamic* Material, FName ParameterName, FVector2D Value);

	/**
	 * Sets <Prefix>CenterUV, <Prefix>ExtentUV and MapRotation on a map material.
	 * Use an empty prefix for a single-layer material, or World and Room for a layered one.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation|Material")
	static void ApplyMapView(UMaterialInstanceDynamic* Material, FVector2D CenterUV, FVector2D ExtentUV, float MapRotation, FName Prefix);

	// Minimap

	/** Pixel position in the minimap frame of a minimap marker position (offset from the centre in radii). */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Minimap")
	static FVector2D MinimapToWidget(FVector2D MarkerPosition, FVector2D FrameSize, float EdgePadding = 0.f);

	/** Degrees clockwise from up pointing from the minimap centre towards a marker, for edge arrows. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Minimap")
	static float GetMinimapEdgeAngle(FVector2D MarkerPosition);

	// Compass

	/** Pixel X on the compass strip of a compass marker position (-1 left edge, 1 right edge). */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Compass")
	static float CompassToWidget(float MarkerPositionX, float StripWidth);

	/**
	 * Compass position (-1..1) of a fixed bearing, e.g. 0 for N or 90 for E, given the view heading.
	 * False when the bearing is outside the compass field of view.
	 */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Compass")
	static bool GetCompassBearingPosition(float Bearing, float Heading, float FieldOfView, float& OutPosition);

	/** U offset and tiling for a strip texture covering 360 degrees with north at U = 0, centred on Heading. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Compass")
	static void GetCompassStripUV(float Heading, float FieldOfView, float& OutOffset, float& OutTiling);

	// World map

	/** Half-size in texture UV the view shows at Zoom (1 = whole layer fits), keeping the image's proportions. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static FVector2D GetWorldMapExtent(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D ViewSize, float Zoom = 1.f);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static FVector2D MapUVToScreen(FVector2D UV, FVector2D PanUV, FVector2D ExtentUV, FVector2D ViewSize);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static FVector2D ScreenToMapUV(FVector2D ScreenPosition, FVector2D PanUV, FVector2D ExtentUV, FVector2D ViewSize);

	/** World location under a point of the view, e.g. for placing a waypoint. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static FVector ScreenToWorld(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D ScreenPosition, FVector2D PanUV,
		FVector2D ExtentUV, FVector2D ViewSize, float Z = 0.f);

	/** Keeps the view inside the image; centres an axis the image already fits on. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static FVector2D ClampMapPan(FVector2D PanUV, FVector2D ExtentUV);

	/** Pan after dragging by DeltaPixels, clamped to the image. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static FVector2D PanMap(FVector2D PanUV, FVector2D DeltaPixels, FVector2D ExtentUV, FVector2D ViewSize);

	/** Zooms to NewZoom keeping the map point under ScreenPosition (e.g. the cursor) in place. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static void ZoomMapAt(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D ViewSize, FVector2D ScreenPosition,
		FVector2D PanUV, FVector2D ExtentUV, float NewZoom, FVector2D& OutPanUV, FVector2D& OutExtentUV);

	/** World map marker closest to ScreenPosition within MaxDistance pixels; on a tie the one drawn on top wins. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|World Map")
	static bool FindMarkerAtScreen(const TArray<FFVMarkerView>& Markers, FVector2D ScreenPosition, FVector2D PanUV,
		FVector2D ExtentUV, FVector2D ViewSize, float MaxDistance, FFVMarkerView& OutMarker);

	// Markers

	/** Icon and tint to draw; undiscovered markers use their Discovery fragment's Undiscovered Icon when it has one. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker")
	static void GetMarkerAppearance(const FFVMarkerView& Marker, TSoftObjectPtr<UTexture2D>& OutIcon, FLinearColor& OutTint);

	/** "85 m" or "1.2 km" from a distance in centimetres. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker")
	static FText FormatDistance(float Distance);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker")
	static bool IsValidMarker(FFVMarkerHandle Marker) { return Marker.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation|Marker", meta = (DisplayName = "Equal (Marker Handle)", CompactNodeTitle = "=="))
	static bool EqualMarker(FFVMarkerHandle A, FFVMarkerHandle B) { return A == B; }
};
