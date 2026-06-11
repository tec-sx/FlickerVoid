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

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TSoftClassPtr<UGameplayEffect> SanityDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TSoftClassPtr<UGameplayEffect> SanityRecoveryEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TSoftClassPtr<UGameplayEffect> WithdrawalEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Effects")
	TMap<FGameplayTag, TSoftClassPtr<UGameplayEffect>> SubstanceEffects;

	// ========================================================================
	// NARRATIVE THRESHOLDS
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SanityDisturbedThreshold = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SanityUnstableThreshold = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SanityProgressionBlockThreshold = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Narrative Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AddictionDependencyThreshold = 0.5f;

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