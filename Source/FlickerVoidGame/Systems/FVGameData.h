// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "NativeGameplayTags.h"

#include "FVGameData.generated.h"

class UGameplayEffect;
class UFVQuestDefinition;
class UObject;

/**
 * UFVGameData
 *
 *	Non-mutable data asset that contains global game data.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "FlickerVoid Game Data", ShortTooltip = "Data asset containing global game data."))
class FLICKERVOID_API UFVGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Returns the loaded game data.
	static const UFVGameData& Get();

	// ========================================================================
	// GAMEPLAY EFFECTS
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;

	// ========================================================================
	// NARRATIVE EFFECTS
	// ========================================================================

	/** Gameplay effect for sanity damage */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TSoftClassPtr<UGameplayEffect> SanityDamageEffect;

	/** Gameplay effect for sanity recovery */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TSoftClassPtr<UGameplayEffect> SanityRecoveryEffect;

	/** Gameplay effect applied during withdrawal */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TSoftClassPtr<UGameplayEffect> WithdrawalEffect;

	/** Gameplay effect for substance consumption */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TMap<FGameplayTag, TSoftClassPtr<UGameplayEffect>> SubstanceEffects;

	// ========================================================================
	// NARRATIVE THRESHOLDS
	// ========================================================================

	/** Sanity threshold to enter "Disturbed" state */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SanityDisturbedThreshold = 0.6f;

	/** Sanity threshold to enter "Unstable" state */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SanityUnstableThreshold = 0.3f;

	/** Sanity threshold below which progression is blocked */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SanityProgressionBlockThreshold = 0.2f;

	/** Addiction level that triggers dependency state */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AddictionDependencyThreshold = 0.5f;

	/** Days clean required to exit "Recovering" state */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds")
	float DaysCleanForRecovery = 7.0f;

	// ========================================================================
	// STARTING QUESTS
	// ========================================================================

	/** Quests available at game start */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Startup")
	TArray<TSoftObjectPtr<UFVQuestDefinition>> InitialQuests;

	/** Quest to auto-start at game begin (main story hook) */
	UPROPERTY(EditDefaultsOnly, Category = "Narrative Startup")
	TSoftObjectPtr<UFVQuestDefinition> InitialMainQuest;
};