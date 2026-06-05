// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "FVQuestDefinition.generated.h"

class UFVQuestObjective;
class UGameplayEffect;

/**
 * EFVQuestCompletionType
 * How the quest objectives should be evaluated for completion
 */
UENUM(BlueprintType)
enum class EFVQuestCompletionType : uint8
{
	/** All objectives must be completed */
	AllObjectives,
	/** Any single objective completes the quest */
	AnyObjective,
	/** A specific number of objectives must be completed */
	MinimumObjectives,
	/** Custom logic defined in Blueprint */
	Custom
};

/**
 * FFVQuestPrerequisite
 * Conditions that must be met before a quest becomes available
 */
USTRUCT(BlueprintType)
struct FFVQuestPrerequisite
{
	GENERATED_BODY()

	/** Quests that must be completed first */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	TArray<FPrimaryAssetId> RequiredCompletedQuests;

	/** World state tags that must be present */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	FGameplayTagContainer RequiredWorldStateTags;

	/** World state tags that must NOT be present */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	FGameplayTagContainer BlockingWorldStateTags;

	/** Minimum sanity required to start this quest */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumSanity = 0.0f;

	/** If true, cannot start while in active addiction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	bool bRequiresCleanState = false;

	/** Minimum trust level with specific NPCs (NPC ID -> Trust Level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	TMap<FName, float> RequiredNPCTrust;

	/** Memory fragments that must be discovered first */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	TArray<FPrimaryAssetId> RequiredMemories;
};

/**
 * FFVQuestConsequence
 * Effects applied when quest state changes
 */
USTRUCT(BlueprintType)
struct FFVQuestConsequence
{
	GENERATED_BODY()

	/** Tags to add to world state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	FGameplayTagContainer WorldStateTags;

	/** Gameplay effects to apply to the protagonist */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

	/** NPC relationship changes (NPC ID -> Delta) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TMap<FName, float> RelationshipChanges;

	/** Memory fragments to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FPrimaryAssetId> MemoriesToUnlock;

	/** Quests to make available */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FPrimaryAssetId> QuestsToUnlock;

	/** Quests to fail/block permanently */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FPrimaryAssetId> QuestsToBlock;

	/** Sanity impact (positive = gain, negative = loss) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SanityImpact = 0.0f;
};

/**
 * UFVQuestDefinition
 * 
 * Data asset defining a quest's structure, objectives, prerequisites, and consequences.
 * This is the "template" - instances are created at runtime to track progress.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "FV Quest Definition"))
class FLICKERVOIDNARRATIVE_API UFVQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVQuestDefinition();

	// ========================================================================
	// IDENTIFICATION
	// ========================================================================

	/** Unique identifier for this quest */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName QuestId;

	/** Display name shown to player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Quest description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	/** Quest type (Main Story, Side, Memory, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag QuestType;

	/** Additional tags for categorization/filtering */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer QuestTags;

	/** Story chapter this quest belongs to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag ChapterTag;

	// ========================================================================
	// OBJECTIVES
	// ========================================================================

	/** All objectives in this quest */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Objectives")
	TArray<TObjectPtr<UFVQuestObjective>> Objectives;

	/** How objectives are evaluated for completion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives")
	EFVQuestCompletionType CompletionType = EFVQuestCompletionType::AllObjectives;

	/** For MinimumObjectives type: how many must be completed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives", meta = (EditCondition = "CompletionType == EFVQuestCompletionType::MinimumObjectives"))
	int32 MinimumObjectivesRequired = 1;

	// ========================================================================
	// PREREQUISITES
	// ========================================================================

	/** Conditions that must be met to start this quest */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prerequisites")
	FFVQuestPrerequisite Prerequisites;

	// ========================================================================
	// CONSEQUENCES
	// ========================================================================

	/** Applied when quest becomes active */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnStartConsequences;

	/** Applied when quest is completed successfully */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnCompleteConsequences;

	/** Applied when quest is failed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnFailConsequences;

	/** Applied when quest is abandoned by player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnAbandonConsequences;

	// ========================================================================
	// SETTINGS
	// ========================================================================

	/** Can this quest be tracked in the journal? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bTrackable = true;

	/** Can the player abandon this quest? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bCanAbandon = true;

	/** Does this quest have a time limit? (in game hours) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float TimeLimitHours = 0.0f;

	/** Is this quest repeatable after completion? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bRepeatable = false;

	/** Priority for quest log sorting (higher = more important) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	int32 Priority = 0;

	// ========================================================================
	// ASSET MANAGEMENT
	// ========================================================================

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};