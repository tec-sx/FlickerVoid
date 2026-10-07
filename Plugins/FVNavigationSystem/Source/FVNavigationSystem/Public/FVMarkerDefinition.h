#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDefinition.h"
#include "Data/FVFragment.h"
#include "FVMarkerDefinition.generated.h"

class UTexture2D;

/** Base for marker fragments. A marker appears in a view only when it has that view's fragment. */
USTRUCT(BlueprintType, meta = (Hidden))
struct FVNAVIGATIONSYSTEM_API FFVMarkerFragment : public FFVFragment
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType, meta = (DisplayName = "Minimap"))
struct FVNAVIGATIONSYSTEM_API FFVMarkerFragment_Minimap : public FFVMarkerFragment
{
	GENERATED_BODY()

	/** Pin the marker to the minimap edge while it is out of range. Tracked markers always do. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	bool bClampToEdge = false;

	/** 0 shows it at any distance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", meta = (ClampMin = 0, Units = "cm"))
	float MaxDistance = 0.f;

	/** Turn the icon with its actor, e.g. a vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	bool bRotateWithActor = false;
};

USTRUCT(BlueprintType, meta = (DisplayName = "World Map"))
struct FVNAVIGATIONSYSTEM_API FFVMarkerFragment_WorldMap : public FFVMarkerFragment
{
	GENERATED_BODY()

	/** Hidden below this world map zoom, so minor places appear as the player zooms in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Map", meta = (ClampMin = 0))
	float MinZoom = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Map")
	bool bRotateWithActor = false;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Compass"))
struct FVNAVIGATIONSYSTEM_API FFVMarkerFragment_Compass : public FFVMarkerFragment
{
	GENERATED_BODY()

	/** 0 shows it at any distance. Tracked markers ignore it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass", meta = (ClampMin = 0, Units = "cm"))
	float MaxDistance = 0.f;
};

/** A place the player discovers by coming close; the discovery is kept in the marker component's fact. */
USTRUCT(BlueprintType, meta = (DisplayName = "Discovery"))
struct FVNAVIGATIONSYSTEM_API FFVMarkerFragment_Discovery : public FFVMarkerFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Discovery", meta = (ClampMin = 0, Units = "cm"))
	float Radius = 2000.f;

	/** Otherwise it shows as undiscovered, with UndiscoveredIcon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Discovery")
	bool bHiddenUntilDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Discovery", meta = (EditCondition = "!bHiddenUntilDiscovered"))
	TSoftObjectPtr<UTexture2D> UndiscoveredIcon;

	/** Applied with the discoverer as Instigator and the marker's actor as Target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Discovery")
	FFVEffectList OnDiscovered;
};

/**
 * A kind of map marker: shop, quest target, fast travel point, waypoint. Icon and name come from Display,
 * the category from Tags, and the views it appears in from its fragments.
 */
UCLASS(BlueprintType)
class FVNAVIGATIONSYSTEM_API UFVMarkerDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Higher draws on top. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	int32 Priority = 0;

	/** Checked with the player as Instigator and the marker's actor as Target, again whenever a fact changes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	FFVConditionSet VisibleWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVMarkerFragment>> Fragments;

	template <typename T>
	const T* FindFragment() const { return FVFragments::Find<T>(Fragments); }
};
