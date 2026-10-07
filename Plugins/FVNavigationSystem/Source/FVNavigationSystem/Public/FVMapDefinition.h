#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Data/FVDefinition.h"
#include "Data/FVFragment.h"
#include "FVMapDefinition.generated.h"

class UTexture2D;

/** Base for map fragments. */
USTRUCT(BlueprintType, meta = (Hidden))
struct FVNAVIGATIONSYSTEM_API FFVMapFragment : public FFVFragment
{
	GENERATED_BODY()
};

/**
 * One image of a map and the world area it covers, e.g. a floor of a building or the ground level of a city.
 * The image top is north (+X) and its right edge east (+Y), as a map capture renders it.
 */
USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVMapLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	FVector2D WorldMin = FVector2D(-50000.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	FVector2D WorldMax = FVector2D(50000.f);

	/** Only covers locations between MinZ and MaxZ, so floors of one building can share an area. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer")
	bool bLimitHeight = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (EditCondition = "bLimitHeight"))
	float MinZ = -1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layer", meta = (EditCondition = "bLimitHeight"))
	float MaxZ = 1000.f;

	bool ContainsXY(const FVector& Location) const;
	bool Contains(const FVector& Location) const;
	FVector2D GetWorldSize() const { return WorldMax - WorldMin; }
	FVector2D WorldToUV(const FVector& Location) const;
	FVector UVToWorld(const FVector2D& UV, float Z) const;
};

/** A map of an area: one or more layers, used while the player stands inside one of them. */
UCLASS(BlueprintType)
class FVNAVIGATIONSYSTEM_API UFVMapDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Higher wins where maps overlap, so an interior map replaces the city map around it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	int32 Priority = 0;

	/** The map is only used while these hold, e.g. once the player owns it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	FFVConditionSet AvailableWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map")
	TArray<FFVMapLayer> Layers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVMapFragment>> Fragments;

	template <typename T>
	const T* FindFragment() const { return FVFragments::Find<T>(Fragments); }

	/** Layer covering Location; a height-limited layer wins over an unlimited one. INDEX_NONE when none does. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	int32 FindLayerAt(const FVector& Location) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	int32 FindLayerByName(FName LayerName) const;

	const FFVMapLayer* GetLayer(int32 LayerIndex) const { return Layers.IsValidIndex(LayerIndex) ? &Layers[LayerIndex] : nullptr; }

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
