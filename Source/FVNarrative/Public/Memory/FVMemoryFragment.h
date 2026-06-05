// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "FVMemoryFragment.generated.h"

/**
 * UFVMemoryFragment
 * 
 * Data asset representing a piece of the protagonist's lost memory.
 * Memories are discovered through gameplay and contribute to identity recovery.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "FV Memory Fragment"))
class FLICKERVOIDNARRATIVE_API UFVMemoryFragment : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVMemoryFragment();

	// ========================================================================
	// IDENTIFICATION
	// ========================================================================

	/** Unique memory identifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName MemoryId;

	/** Display title */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText Title;

	/** Memory description/content */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Content;

	/** Memory type (Identity, Relationship, Trauma, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag MemoryType;

	/** Initial clarity when discovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag InitialClarity;

	// ========================================================================
	// NARRATIVE IMPACT
	// ========================================================================

	/** How much this contributes to identity recovery (0-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IdentityContribution = 0.05f;

	/** Sanity impact when discovered (can be negative for trauma) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SanityImpact = 0.0f;

	/** World state tags to apply when discovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	FGameplayTagContainer GrantedWorldStateTags;

	/** Quests unlocked by this memory */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TArray<FPrimaryAssetId> UnlockedQuests;

	// ========================================================================
	// DISCOVERY
	// ========================================================================

	/** If set, requires minimum sanity to properly recall */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Discovery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumSanityForClarity = 0.0f;

	/** Memories that must be discovered first */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Discovery")
	TArray<FPrimaryAssetId> PrerequisiteMemories;

	// ========================================================================
	// PRESENTATION
	// ========================================================================

	/** Visual/audio asset to play when memory is discovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UObject> MemorySequenceAsset;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};