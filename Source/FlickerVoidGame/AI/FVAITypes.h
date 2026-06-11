// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVAITypes.generated.h"

class UCharacterAttributeComponent;

// ============================================================================
// NOTE: Character states, behavior decisions, attack styles, and response types
// are all represented as FGameplayTag using the tags defined in FVAITags.h:
//
// Character States:     Character_State_Idle, Character_State_Alert, etc.
// Behavior Decisions:   Behavior_Decision_Ignore, Behavior_Decision_Greet, etc.
// Attack Styles:        Attack_Style_CalculatedStrike, etc.
// Response Types:       Response_Defend, Response_Attack, Response_Intimidate, etc.
//
// Use FVAITags:: namespace to access these tags.
// ============================================================================

/**
 * Parameters for an attack - uses FGameplayTag for attack style
 */
USTRUCT(BlueprintType)
struct FAttackParams
{
	GENERATED_BODY()

	/** Attack style tag (e.g., Attack_Style_CalculatedStrike) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attack", meta = (Categories = "Attack.Style"))
	FGameplayTag AttackStyle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attack")
	float Damage = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attack")
	float DefenseModifier = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attack")
	float AttackSpeed = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attack")
	bool bIsSneakAttack = false;
};

// ============================================================================
// PLAYER RESPONSE SYSTEM
// ============================================================================

/**
 * A response option available to the player - uses FGameplayTag for response type
 */
USTRUCT(BlueprintType)
struct FResponseOption
{
	GENERATED_BODY()

	/** Response type tag (e.g., Response_Defend, Response_Persuade) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response", meta = (Categories = "Response"))
	FGameplayTag ResponseType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response")
	FText DisplayText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response")
	float SuccessChance = 0.5f;

	/** Required attribute tags for this response */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response")
	TArray<FName> RequiredSkills;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response")
	float SkillThreshold = 0.0f;

	/** Tags required for this response to be available */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response")
	FGameplayTagContainer RequiredTags;

	/** Tags that block this response */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Response")
	FGameplayTagContainer BlockingTags;
};

/**
 * Result of executing a player response
 */
USTRUCT(BlueprintType)
struct FResponseResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result")
	FText ResultMessage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result")
	float RelationshipChange = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result")
	TMap<FName, float> AttributeChanges;

	/** Tags to add to the world state on this result */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result")
	FGameplayTagContainer TagsToAdd;

	/** Tags to remove from the world state on this result */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result")
	FGameplayTagContainer TagsToRemove;

	/** The resulting NPC behavior decision tag */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Result", meta = (Categories = "Behavior.Decision"))
	FGameplayTag NPCReactionTag;
};
