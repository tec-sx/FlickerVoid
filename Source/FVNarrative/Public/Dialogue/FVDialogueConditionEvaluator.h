// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVDialogueTypes.h"

#include "FVDialogueConditionEvaluator.generated.h"

class UFVWorldStateSubsystem;
class UFVQuestSubsystem;
class UAbilitySystemComponent;

/**
 * UFVDialogueConditionEvaluator
 * 
 * Parses and evaluates dialogue conditions.
 * Supports both referenced conditions (by ID) and inline conditions (string format).
 */
UCLASS(BlueprintType)
class FLICKERVOIDNARRATIVE_API UFVDialogueConditionEvaluator : public UObject
{
	GENERATED_BODY()

public:
	UFVDialogueConditionEvaluator();

	/** Initialize with required subsystems */
	void Initialize(UWorld* World);

	/**
	 * Parse an inline condition string
	 * Format: "Type:Key:Comparison:Value" or "Type:Key:Present/Absent"
	 * Examples:
	 * - "WorldState:Chapter1:Present"
	 * - "Relationship:NPC_Barkeeper:>=:0.5"
	 * - "Attribute:Sanity:>=:0.3"
	 * - "Quest:MainQuest_01:Completed"
	 * - "Memory:Memory_Childhood:Discovered"
	 * - "Tag:Character.Trait.Aggressive:Present"
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conditions")
	FFVDialogueCondition ParseInlineCondition(const FString& ConditionString) const;

	/** Evaluate a single condition */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conditions")
	bool EvaluateCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;

	/** Evaluate multiple conditions with an operator */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conditions")
	bool EvaluateConditions(const TArray<FFVDialogueCondition>& Conditions, EFVConditionOperator Operator, const FFVDialogueContext& Context) const;

	/** Evaluate a dialogue line's conditions (both referenced and inline) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conditions")
	bool EvaluateLineConditions(const FFVDialogueLine& Line, const FFVDialogueContext& Context, const UDataTable* ConditionsTable) const;

	/** Calculate specificity score for a line (how specific/relevant it is) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conditions")
	float CalculateSpecificityScore(const FFVDialogueLine& Line, const FFVDialogueContext& Context, const UDataTable* ConditionsTable) const;

private:
	UPROPERTY()
	TWeakObjectPtr<UFVWorldStateSubsystem> WorldStateSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UFVQuestSubsystem> QuestSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UWorld> CachedWorld;

	bool EvaluateWorldStateCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;
	bool EvaluateRelationshipCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;
	bool EvaluateAttributeCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;
	bool EvaluateQuestCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;
	bool EvaluateTagCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;
	bool EvaluateMemoryCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;
	bool EvaluateCustomCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const;

	bool CompareValues(float Value, EFVConditionComparison Comparison, float CompareValue) const;
};