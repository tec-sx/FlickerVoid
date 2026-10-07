#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVNavigationTypes.h"
#include "GameplayTagContainer.h"
#include "FVNavigatorComponent.generated.h"

class UFVMapDefinition;
struct FFVMarker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnActiveMapChanged, UFVMapDefinition*, Map, int32, LayerIndex);

/**
 * The player's view of the navigation data. Put it on the player pawn or player controller: it follows the map underfoot,
 * discovers nearby places and builds the minimap, compass and world map views the HUD draws.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVNAVIGATIONSYSTEM_API UFVNavigatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVNavigatorComponent();

	static UFVNavigatorComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	UFVMapDefinition* GetActiveMap() const { return ActiveMap; }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	int32 GetActiveLayer() const { return ActiveLayer; }

	/** The actor the maps follow: the owner, or the possessed pawn when the owner is a controller. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	AActor* GetViewActor() const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FVector GetViewLocation() const;

	/** Camera yaw when the owner is a controlled pawn, otherwise the owner's yaw. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	float GetViewYaw() const;

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void SetMinimapRadius(float Radius);

	/** Multiplies the minimap radius; below 1 zooms in. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void ZoomMinimap(float Factor);

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void SetCategoryHidden(FGameplayTag Category, bool bHidden);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool IsCategoryHidden(FGameplayTag Category) const { return HiddenCategories.HasTagExact(Category); }

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	FFVMinimapView BuildMinimapView() const;

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	FFVCompassView BuildCompassView() const;

	/** Zoom is the world map's own zoom (1 = whole layer) and only hides markers below their MinZoom. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	FFVWorldMapView BuildWorldMapView(UFVMapDefinition* Map, int32 LayerIndex, float Zoom = 1.f) const;

	/** Checks the map underfoot and nearby undiscovered places now rather than on the next update. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void UpdateNavigation();

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnActiveMapChanged OnActiveMapChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navigation")
	FFVMinimapSettings Minimap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navigation")
	FFVCompassSettings Compass;

	/** Marker categories (definition tags) left out of every view, e.g. switched off in the map legend. Tracked markers still show. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navigation")
	FGameplayTagContainer HiddenCategories;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateActiveMap();
	void DiscoverNearbyMarkers();
	bool PassesFilters(const FFVMarker& Marker, bool bTracked) const;
	void FillView(FFVMarkerView& View, const FFVMarker& Marker, const FVector& ViewLocation, bool bTracked) const;
	float GetOwnerYaw() const;

	UPROPERTY(Transient)
	TObjectPtr<UFVMapDefinition> ActiveMap;

	int32 ActiveLayer = INDEX_NONE;
};
