// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"

#include "FVQuestObjective.generated.h"

/**
 * EFVObjectiveState
 */
UENUM(BlueprintType)
enum class EFVObjectiveState : uint8
{
	Locked,
	Active,
	Completed,
	Failed,
	Skipped
};

/**
 * UFVQuestObjective
 * 
 * Base class for quest objectives. Subclass for specific objective types.
 * Objectives are instanced within quest definitions and tracked at runtime.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, Abstract)
class FLICKERVOIDNARRATIVE_API UFVQuestObjective : public UObject
{
	GENERATED_BODY()

public:
	UFVQuestObjective();

	// ========================================================================
	// IDENTIFICATION
	// ========================================================================

	/** Unique ID within the quest */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName ObjectiveId;

	/** Display text shown to player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayText;

	/** Optional hint text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText HintText;

	/** Objective type tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FGameplayTag ObjectiveType;

	// ========================================================================
	// FLOW CONTROL
	// ========================================================================

	/** Objectives that must be completed before this one becomes active */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	TArray<FName> RequiredObjectiveIds;

	/** If true, this objective is optional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	bool bOptional = false;

	/** If true, this objective is hidden until it becomes active */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	bool bHiddenUntilActive = false;

	// ========================================================================
	// TRACKING
	// ========================================================================

	/** For countable objectives: target count */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking")
	int32 TargetCount = 1;

	// ========================================================================
	// VIRTUAL METHODS
	// ========================================================================

	/** Called to check if this objective is complete (override in subclasses) */
	UFUNCTION(BlueprintNativeEvent, Category = "Objective")
	bool CheckCompletion(const class UFVQuestInstance* QuestInstance) const;

	/** Get progress as 0-1 value */
	UFUNCTION(BlueprintNativeEvent, Category = "Objective")
	float GetProgress(const class UFVQuestInstance* QuestInstance) const;

	/** Get localized progress text */
	UFUNCTION(BlueprintNativeEvent, Category = "Objective")
	FText GetProgressText(const class UFVQuestInstance* QuestInstance) const;
};

// ============================================================================
// OBJECTIVE SUBCLASSES
// ============================================================================

/**
 * UFVObjective_TalkTo
 * Talk to a specific NPC
 */
UCLASS(BlueprintType, Meta = (DisplayName = "Talk To NPC"))
class FLICKERVOIDNARRATIVE_API UFVObjective_TalkTo : public UFVQuestObjective
{
	GENERATED_BODY()

public:
	UFVObjective_TalkTo();

	/** The NPC to talk to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FName TargetNPCId;

	/** Specific dialogue node that must be reached (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FName RequiredDialogueNodeId;

	virtual bool CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const override;
};

/**
 * UFVObjective_ObtainItem
 * Obtain a specific item or items
 */
UCLASS(BlueprintType, Meta = (DisplayName = "Obtain Item"))
class FLICKERVOIDNARRATIVE_API UFVObjective_ObtainItem : public UFVQuestObjective
{
	GENERATED_BODY()

public:
	UFVObjective_ObtainItem();

	/** Item to obtain */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FPrimaryAssetId ItemAssetId;

	/** Quantity required */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	int32 Quantity = 1;

	virtual bool CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const override;
	virtual float GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const override;
};

/**
 * UFVObjective_ReachLocation
 * Reach a specific area/location
 */
UCLASS(BlueprintType, Meta = (DisplayName = "Reach Location"))
class FLICKERVOIDNARRATIVE_API UFVObjective_ReachLocation : public UFVQuestObjective
{
	GENERATED_BODY()

public:
	UFVObjective_ReachLocation();

	/** Location identifier (matches trigger volumes) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FName LocationId;

	virtual bool CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const override;
};

/**
 * UFVObjective_ReachThreshold
 * Reach a specific attribute/relationship threshold
 */
UCLASS(BlueprintType, Meta = (DisplayName = "Reach Threshold"))
class FLICKERVOIDNARRATIVE_API UFVObjective_ReachThreshold : public UFVQuestObjective
{
	GENERATED_BODY()

public:
	UFVObjective_ReachThreshold();

	/** Type of threshold to check */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FGameplayTag ThresholdType;

	/** For NPC relationship thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FName TargetNPCId;

	/** For attribute thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FGameplayAttribute TargetAttribute;

	/** The threshold value to reach */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	float ThresholdValue = 0.5f;

	/** Should be >= (true) or <= (false) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	bool bGreaterThanOrEqual = true;

	virtual bool CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const override;
	virtual float GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const override;
};

/**
 * UFVObjective_WaitTime
 * Wait for a period of game time
 */
UCLASS(BlueprintType, Meta = (DisplayName = "Wait Time"))
class FLICKERVOIDNARRATIVE_API UFVObjective_WaitTime : public UFVQuestObjective
{
	GENERATED_BODY()

public:
	UFVObjective_WaitTime();

	/** Hours to wait (game time) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	float WaitHours = 1.0f;

	virtual bool CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const override;
	virtual float GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const override;
};

/**
 * UFVObjective_MaintainState
 * Maintain a state for a duration (e.g., stay clean for 3 days)
 */
UCLASS(BlueprintType, Meta = (DisplayName = "Maintain State"))
class FLICKERVOIDNARRATIVE_API UFVObjective_MaintainState : public UFVQuestObjective
{
	GENERATED_BODY()

public:
	UFVObjective_MaintainState();

	/** Tag that must remain present/absent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FGameplayTag StateTag;

	/** Should the tag be present (true) or absent (false) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	bool bRequireTagPresent = true;

	/** Duration in game hours */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	float DurationHours = 24.0f;

	virtual bool CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const override;
	virtual float GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const override;
};