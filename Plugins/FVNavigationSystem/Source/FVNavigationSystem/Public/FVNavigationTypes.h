#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVNavigationTypes.generated.h"

class AActor;
class UFVMapDefinition;
class UFVMarkerDefinition;

/** Identifies a marker registered with the navigation subsystem. */
USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVMarkerHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Id = INDEX_NONE;

	bool IsValid() const { return Id != INDEX_NONE; }
	void Reset() { Id = INDEX_NONE; }

	bool operator==(const FFVMarkerHandle& Other) const { return Id == Other.Id; }
	bool operator!=(const FFVMarkerHandle& Other) const { return Id != Other.Id; }
	friend uint32 GetTypeHash(const FFVMarkerHandle& Handle) { return ::GetTypeHash(Handle.Id); }
};

UENUM(BlueprintType)
enum class EFVMarkerElevation : uint8
{
	Level,
	Above,
	Below
};

/**
 * A marker as one view presents it.
 * Minimap: Position is the offset from the centre in minimap radii (+X right, +Y down).
 * World map: Position is the texture UV of the layer.
 * Compass: Position.X runs from -1 (left edge) to 1 (right edge) and Rotation is the bearing relative to the view.
 */
USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVMarkerView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FFVMarkerHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	TObjectPtr<UFVMarkerDefinition> Definition;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FText Label;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FVector2D Position = FVector2D::ZeroVector;

	/** Degrees clockwise from up. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float Rotation = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float Distance = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	EFVMarkerElevation Elevation = EFVMarkerElevation::Level;

	/** Out of range and pinned to the edge of the view. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	bool bClamped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	bool bDiscovered = true;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	bool bTracked = false;
};

USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVMinimapView
{
	GENERATED_BODY()

	/** Map under the player, or null when they stand outside every map. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	TObjectPtr<UFVMapDefinition> Map;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	int32 LayerIndex = INDEX_NONE;

	/** Layer texture UV under the player. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FVector2D CenterUV = FVector2D(0.5f);

	/** Half of the texture area the minimap shows, in UV. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FVector2D ExtentUV = FVector2D(0.5f);

	/** Degrees clockwise to rotate the map image by. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float MapRotation = 0.f;

	/** Degrees clockwise from up for the player icon. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float PlayerRotation = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	TArray<FFVMarkerView> Markers;
};

USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVCompassView
{
	GENERATED_BODY()

	/** Degrees from north (+X), clockwise, 0..360. */
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float Heading = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	TArray<FFVMarkerView> Markers;
};

USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVWorldMapView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	TObjectPtr<UFVMapDefinition> Map;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	int32 LayerIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	bool bPlayerOnLayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	FVector2D PlayerUV = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float PlayerRotation = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	TArray<FFVMarkerView> Markers;
};

USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVMinimapSettings
{
	GENERATED_BODY()

	/** World distance from the centre to the edge of the minimap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", meta = (ClampMin = 100))
	float Radius = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", meta = (ClampMin = 100))
	float MinRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", meta = (ClampMin = 100))
	float MaxRadius = 20000.f;

	/** Turn the map with the camera so the view direction points up; otherwise north stays up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	bool bRotateWithView = true;
};

USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVCompassSettings
{
	GENERATED_BODY()

	/** Degrees of the world the compass strip covers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass", meta = (ClampMin = 10, ClampMax = 360))
	float FieldOfView = 180.f;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Navigation"))
class FVNAVIGATIONSYSTEM_API UFVNavigationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFVNavigationSettings();

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	static const UFVNavigationSettings& Get() { return *GetDefault<UFVNavigationSettings>(); }

	/** Maps the player can stand on. Where they overlap, the highest priority wins. */
	UPROPERTY(Config, EditAnywhere, Category = "Maps")
	TArray<TSoftObjectPtr<UFVMapDefinition>> Maps;

	/** Marker used for the waypoint the player places on the world map. */
	UPROPERTY(Config, EditAnywhere, Category = "Markers")
	TSoftObjectPtr<UFVMarkerDefinition> WaypointMarker;

	/** Height difference beyond which a marker reads as above or below the player. */
	UPROPERTY(Config, EditAnywhere, Category = "Markers", meta = (ClampMin = 0, Units = "cm"))
	float ElevationThreshold = 300.f;

	/** Seconds between the navigator's checks for the map underfoot and nearby undiscovered places. */
	UPROPERTY(Config, EditAnywhere, Category = "Markers", meta = (ClampMin = 0.02, Units = "s"))
	float UpdateInterval = 0.25f;

	/** Folder new map captures are saved to. */
	UPROPERTY(Config, EditAnywhere, Category = "Capture", meta = (ContentDir))
	FDirectoryPath CaptureFolder;

	/** Captures leave these actors out, e.g. characters and vehicles. */
	UPROPERTY(Config, EditAnywhere, Category = "Capture")
	TArray<TSoftClassPtr<AActor>> CaptureIgnoredClasses;

	/** Captures leave out actors with any of these tags. */
	UPROPERTY(Config, EditAnywhere, Category = "Capture")
	TArray<FName> CaptureIgnoredActorTags;
};
