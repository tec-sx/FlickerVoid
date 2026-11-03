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

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (DefaultToSelf = "Actor", GameplayTagFilter = "Relevance", ToolTip = "The actor's relevance component for the group, if it has one."))
	static UFVRelevanceComponent* Find(const AActor* Actor, const FGameplayTag& InGroup);

	UFUNCTION(BlueprintCallable, Category = "FV|Relevance", meta = (DefaultToSelf = "Actor", GameplayTagFilter = "Relevance", ToolTip = "The actor's relevance component for the group, added at runtime when missing, so systems need no extra setup on their actors."))
	static UFVRelevanceComponent* FindOrAdd(AActor* Actor, const FGameplayTag& InGroup);

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (ToolTip = "Group whose thresholds this component uses."))
	FGameplayTag GetGroup() const { return Group; }

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (ToolTip = "Band at the last update; Far until the first update."))
	EFVRelevanceBand GetBand() const { return Band; }

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (ToolTip = "True while the owner is in the group's Near band."))
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

	float ScoreViewpoint(const FTransform& Viewpoint) const;
	void UpdateBand(float NewDistance);
	bool HasPendingBandChange() const { return Band != BroadcastBand; }
	void BroadcastBandChange();

	FFVRelevanceGroup GroupSettings;
	EFVRelevanceBand Band = EFVRelevanceBand::Far;
	EFVRelevanceBand BroadcastBand = EFVRelevanceBand::Far;
	float Distance = TNumericLimits<float>::Max();
	bool bRegistered = false;
};
