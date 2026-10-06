#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FVNavigationTypes.h"
#include "GameplayTagContainer.h"
#include "FVMapMarkerComponent.generated.h"

class UFVMarkerDefinition;
class UFVMapMarkerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnMapMarkerEvent, UFVMapMarkerComponent*, Marker);

/** Puts its actor on the map, minimap and compass, as its marker definition describes. Marks its own location. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVNAVIGATIONSYSTEM_API UFVMapMarkerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFVMapMarkerComponent();

	static UFVMapMarkerComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	UFVMarkerDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void SetDefinition(UFVMarkerDefinition* NewDefinition);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool IsMarkerEnabled() const { return bMarkerEnabled; }

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void SetMarkerEnabled(bool bEnabled);

	/** Label, or the definition's name when Label is empty. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FText GetLabel() const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FFVMarkerHandle GetHandle() const { return Handle; }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FGameplayTag GetDiscoveredFact() const { return DiscoveredFact; }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool IsDiscovered() const;

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	bool Discover(AActor* Discoverer);

	UPROPERTY(BlueprintAssignable, Category = "FV|Navigation")
	FFVOnMapMarkerEvent OnDiscovered;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker")
	TObjectPtr<UFVMarkerDefinition> Definition;

	/** Name of this particular place, e.g. the inn's name; empty uses the definition's name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker")
	FText Label;

	/** Fact set once the player discovers this place. Needed by markers with a Discovery fragment. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker", meta = (Categories = "Fact"))
	FGameplayTag DiscoveredFact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Marker")
	bool bMarkerEnabled = true;

private:
	void Register();
	void Unregister();

	FFVMarkerHandle Handle;
};
