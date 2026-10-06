#pragma once

#include "CoreMinimal.h"
#include "FVNavigationTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVNavigationSubsystem.generated.h"

class UFVMapDefinition;
class UFVMapMarkerComponent;
class UFVMarkerDefinition;
class UFVNavigatorComponent;
struct FFVConditionContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnMarkerEvent, FFVMarkerHandle, Marker);

/** A registered marker: on an actor through its marker component, or at a fixed location. */
USTRUCT()
struct FVNAVIGATIONSYSTEM_API FFVMarker
{
	GENERATED_BODY()

	UPROPERTY()
	FFVMarkerHandle Handle;

	UPROPERTY()
	TObjectPtr<UFVMarkerDefinition> Definition;

	UPROPERTY()
	TWeakObjectPtr<UFVMapMarkerComponent> Component;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FText Label;

	bool bDiscovered = true;
	bool bVisible = true;

	FVector GetLocation() const;
	float GetYaw() const;
	AActor* GetActor() const;
	FText GetLabel() const;
};

/** Maps and markers of the world, discovery, the tracked marker and the player's waypoint. Nothing here runs on a timer. */
UCLASS()
class FVNAVIGATIONSYSTEM_API UFVNavigationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVNavigationSubsystem* Get(const UObject* WorldContext);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** Highest-priority available map with a layer covering Location. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	UFVMapDefinition* FindMapAt(const FVector& Location, int32& OutLayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	TArray<UFVMapDefinition*> GetMaps() const;

	/** Marks a location that has no actor, e.g. a quest area or a search zone. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	FFVMarkerHandle AddMarkerAtLocation(UFVMarkerDefinition* Definition, FVector Location, FText Label);

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void RemoveMarker(FFVMarkerHandle Marker);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool IsMarkerValid(FFVMarkerHandle Marker) const { return Markers.Contains(Marker.Id); }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool IsMarkerVisible(FFVMarkerHandle Marker) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool IsMarkerDiscovered(FFVMarkerHandle Marker) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FVector GetMarkerLocation(FFVMarkerHandle Marker) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	UFVMarkerDefinition* GetMarkerDefinition(FFVMarkerHandle Marker) const;

	/** Actor of a component marker; null for location markers. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	AActor* GetMarkerActor(FFVMarkerHandle Marker) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	TArray<FFVMarkerHandle> GetMarkerHandles() const;

	/** Writes the marker's discovered fact and applies its OnDiscovered effects. False if already discovered. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	bool DiscoverMarker(FFVMarkerHandle Marker, AActor* Discoverer);

	/** Re-checks every marker's VisibleWhen and discovery. Runs on its own whenever a fact changes. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void RefreshMarkers();

	/** The marker the player follows; it shows at any distance and stays on the minimap and compass edges. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void SetTrackedMarker(FFVMarkerHandle Marker);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FFVMarkerHandle GetTrackedMarker() const { return TrackedMarker; }

	/** Replaces the player's waypoint and tracks it. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	FFVMarkerHandle SetWaypoint(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void ClearWaypoint();

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FFVMarkerHandle GetWaypoint() const { return Waypoint; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMarkerEvent OnMarkerAdded;

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMarkerEvent OnMarkerRemoved;

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMarkerEvent OnMarkerDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMarkerEvent OnMarkerVisibilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMarkerEvent OnTrackedMarkerChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMarkerEvent OnWaypointChanged;

	FFVMarkerHandle RegisterComponent(UFVMapMarkerComponent* Component);

	/** The player's navigator; its owner is the Instigator of marker conditions and the default discoverer. */
	void SetNavigator(UFVNavigatorComponent* Navigator);
	UFVNavigatorComponent* GetNavigator() const { return Navigator.Get(); }

	const FFVMarker* FindMarker(FFVMarkerHandle Marker) const { return Markers.Find(Marker.Id); }
	const TMap<int32, FFVMarker>& GetMarkers() const { return Markers; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	FFVMarkerHandle AddMarker(FFVMarker&& Marker);
	/** Re-reads discovery and visibility; true when visibility changed. */
	bool UpdateMarker(FFVMarker& Marker);
	bool ReadDiscovered(const FFVMarker& Marker) const;
	FFVConditionContext MakeContext(const FFVMarker& Marker, AActor* Instigator) const;
	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);

	UPROPERTY(Transient)
	TMap<int32, FFVMarker> Markers;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVMapDefinition>> Maps;

	TWeakObjectPtr<UFVNavigatorComponent> Navigator;
	FFVMarkerHandle TrackedMarker;
	FFVMarkerHandle Waypoint;
	int32 NextMarkerId = 0;
	FDelegateHandle FactChangedHandle;
};
