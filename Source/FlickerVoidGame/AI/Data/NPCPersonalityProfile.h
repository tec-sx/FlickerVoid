// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AI/FVAITypes.h"
#include "NPCPersonalityProfile.generated.h"

class UFVCharacterAttributeComponent;

// ============================================================================
// RELATIONSHIP EVALUATION RULES
// ============================================================================

UENUM(BlueprintType)
enum class EEvaluationOperator : uint8
{
	Multiply,
	Add,
	Subtract,
	Min,
	Max
};

USTRUCT(BlueprintType)
struct FAttributeWeight
{
	GENERATED_BODY()

	// Which attribute to consider (e.g., "Reputation", "Notoriety")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetAttribute;

	// How much this attribute matters (-1 to 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Weight = 0.0f;

	// What NPC attribute determines the weight (e.g., "Aggression")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NPCPersonalityModifier;

	// How the modifier affects the weight
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEvaluationOperator Operator = EEvaluationOperator::Multiply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvertModifier = false; // Use (1.0 - value) instead
};

USTRUCT(BlueprintType)
struct FBehaviorThreshold
{
	GENERATED_BODY()

	/** Behavior decision tag (e.g., Behavior_Decision_Attack) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Behavior.Decision"))
	FGameplayTag DecisionTag;

	// Relationship score threshold to trigger this behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RelationshipThreshold = 0.0f;

	// Additional conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> RequiredAttributes; // "Courage" > 0.5, "Intelligence" < 0.3

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Priority = 0.0f; // Higher priority behaviors checked first
};

USTRUCT(BlueprintType)
struct FConditionalModifier
{
	GENERATED_BODY()

	// If this condition is true...
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ConditionAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConditionThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGreaterThan = true; // true = >, false = <

	// ...apply these modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> AttributeModifiers; // "Aggression" -> +0.3
};

USTRUCT(BlueprintType)
struct FAttackStyleRule
{
	GENERATED_BODY()

	/** Attack style tag (e.g., Attack_Style_BerserkerCharge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Attack.Style"))
	FGameplayTag AttackStyleTag;

	// Conditions for this attack style
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> MinAttributes; // "Intelligence" -> 0.5

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> MaxAttributes; // "Intelligence" -> 0.3

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredSkills; // "Ranged" must be > threshold

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillThreshold = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Priority = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description; // For designer reference
};

UCLASS(BlueprintType)
class FLICKERVOID_API UNPCPersonalityProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString ProfileName = "Default";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info", meta = (MultiLine = true))
	FString Description;

	// ========================================================================
	// RELATIONSHIP EVALUATION
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship Evaluation")
	TArray<FAttributeWeight> RelationshipWeights;

	// Base relationship score before any calculations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship Evaluation", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float BaseRelationship = 0.0f;

	// If true, unknown characters start at BaseRelationship, not 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship Evaluation")
	bool bAssumeUnknownCharactersAreNeutral = true;

	// ========================================================================
	// RECOGNITION RULES
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recognition")
	float BaseRecognitionChance = 0.3f;

	// How much target's Notoriety affects recognition (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recognition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NotorietyRecognitionWeight = 0.6f;

	// How much NPC's Intelligence affects recognition (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recognition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IntelligenceRecognitionWeight = 0.4f;

	// ========================================================================
	// BEHAVIOR THRESHOLDS
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	TArray<FBehaviorThreshold> BehaviorThresholds;

	// ========================================================================
	// CONDITIONAL MODIFIERS
	// ========================================================================

	// Dynamic personality changes based on state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditional Behavior")
	TArray<FConditionalModifier> ConditionalModifiers;

	// E.g., "If Intoxication > 0.5, Aggression +0.3, Intelligence -0.2"

	// ========================================================================
	// ATTACK STYLES
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FAttackStyleRule> AttackStyles;

	// ========================================================================
	// METHODS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	float EvaluateRelationship(
		UFVCharacterAttributeComponent* NPCAttributes,
		UFVCharacterAttributeComponent* TargetAttributes) const;

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	bool CheckRecognition(
		UFVCharacterAttributeComponent* NPCAttributes,
		UFVCharacterAttributeComponent* TargetAttributes) const;

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	FGameplayTag DetermineBehavior(
		float RelationshipScore,
		UFVCharacterAttributeComponent* NPCAttributes) const;

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	FGameplayTag SelectAttackStyle(UFVCharacterAttributeComponent* NPCAttributes) const;

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	void ApplyConditionalModifiers(
		UFVCharacterAttributeComponent* NPCAttributes,
		TMap<FName, float>& OutModifiers) const;

private:
	float ApplyAttributeWeight(const FAttributeWeight& WeightRule,
		UFVCharacterAttributeComponent* NPCAttributes,
		UFVCharacterAttributeComponent* TargetAttributes) const;
};
