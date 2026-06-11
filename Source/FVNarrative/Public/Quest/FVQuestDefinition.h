// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "FVQuestDefinition.generated.h"

class UFVQuestObjective;
class UGameplayEffect;


UENUM(BlueprintType)
enum class EFVQuestCompletionType : uint8
{
	AllObjectives,
	AnyObjective,
	MinimumObjectives,
	Custom
};


USTRUCT(BlueprintType)
struct FFVQuestPrerequisite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	TArray<FPrimaryAssetId> RequiredCompletedQuests;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	FGameplayTagContainer RequiredWorldStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	FGameplayTagContainer BlockingWorldStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	TMap<FName, float> RequiredNPCTrust;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
	TArray<FPrimaryAssetId> RequiredMemories;
};

USTRUCT(BlueprintType)
struct FFVQuestConsequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	FGameplayTagContainer WorldStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TMap<FName, float> RelationshipChanges;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FPrimaryAssetId> MemoriesToUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FPrimaryAssetId> QuestsToUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SanityImpact = 0.0f;
};


UCLASS(BlueprintType, Const, Meta = (DisplayName = "FV Quest Definition"))
class FLICKERVOIDNARRATIVE_API UFVQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVQuestDefinition();

	// ========================================================================
	// IDENTIFICATION
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName QuestId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer QuestTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag ChapterTag;

	// ========================================================================
	// OBJECTIVES
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Objectives")
	TArray<TObjectPtr<UFVQuestObjective>> Objectives;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives")
	EFVQuestCompletionType CompletionType = EFVQuestCompletionType::AllObjectives;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnStartConsequences;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnCompleteConsequences;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnFailConsequences;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consequences")
	FFVQuestConsequence OnAbandonConsequences;

	// ========================================================================
	// SETTINGS
	// ========================================================================

	/** Can this quest be tracked in the journal? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bTrackable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bCanAbandon = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float TimeLimitHours = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bRepeatable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	int32 Priority = 0;

	// ========================================================================
	// ASSET MANAGEMENT
	// ========================================================================

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};