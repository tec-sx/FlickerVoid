#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Relevance/FVRelevanceSettings.h"
#include "FVRelevanceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnRelevanceBandChanged, EFVRelevanceBand, NewBand, EFVRelevanceBand, OldBand);

/**
 * Scores its owner by distance to the local players through the Significance Manager and reports
 * which band of its group the owner is in. One component per group; an actor in several groups
 * (interactable and scan target) carries one for each.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVCORERUNTIME_API UFVRelevanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVRelevanceComponent();

	static UFVRelevanceComponent* Find(const AActor* Actor, const FGameplayTag& InGroup);

	/** Finds the actor's component for InGroup, or adds one. Systems call this so their actors need no extra setup. */
	static UFVRelevanceComponent* FindOrAdd(AActor* Actor, const FGameplayTag& InGroup);

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (ToolTip = "Group whose thresholds this component uses."))
	FGameplayTag GetGroup() const { return Group; }

	UFUNCTION(BlueprintPure, Category = "FV|Relevance")
	EFVRelevanceBand GetBand() const { return Band; }

	UFUNCTION(BlueprintPure, Category = "FV|Relevance")
	bool IsNear() const { return Band == EFVRelevanceBand::Near; }

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (ToolTip = "Distance to the closest local player at the last update."))
	float GetDistance() const { return Distance; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Relevance")
	FFVOnRelevanceBandChanged OnBandChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relevance", meta = (Categories = "Relevance", ToolTip = "Group whose thresholds apply, see FlickerVoid > Relevance."))
	FGameplayTag Group;

private:
	friend class UFVRelevanceSubsystem;

	/** Runs on worker threads during the Significance Manager update; reads only the owner's location. */
	float ScoreViewpoint(const FTransform& Viewpoint) const;
	/** Stores the new distance and band; true when the band changed and needs broadcasting. */
	bool UpdateBand(float NewDistance);
	void BroadcastBandChange();

	FFVRelevanceGroup GroupSettings;
	EFVRelevanceBand Band = EFVRelevanceBand::Far;
	EFVRelevanceBand BroadcastBand = EFVRelevanceBand::Far;
	float Distance = TNumericLimits<float>::Max();
	bool bRegistered = false;
};
