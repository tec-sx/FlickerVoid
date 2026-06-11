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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Identity)
	FName MemoryId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Identity)
	FText Title;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Identity, meta = (MultiLine = true))
	FText Content;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Identity)
	FGameplayTag MemoryType;

	// ========================================================================
	// NARRATIVE IMPACT
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Impact, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IdentityContribution = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Impact, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SanityImpact = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Impact)
	FGameplayTagContainer GrantedWorldStateTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Impact)
	TArray<FPrimaryAssetId> UnlockedQuests;

	// ========================================================================
	// DISCOVERY
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Discovery)
	TArray<FPrimaryAssetId> PrerequisiteMemories;

	// ========================================================================
	// PRESENTATION
	// ========================================================================

	/** Visual/audio asset to play when memory is discovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Presentation)
	TSoftObjectPtr<UObject> MemorySequenceAsset;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};