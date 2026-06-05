// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"

#include "FVCharacterDataAsset.generated.h"

class UGameplayEffect;

/**
 * FFVAttributeInitializer
 * 
 * Struct to define initial attribute values in data assets.
 * Game designers can set these in the editor.
 */
USTRUCT(BlueprintType)
struct FFVAttributeInitializer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Value = 0.5f;
};

/**
 * FFVSkillDefinition
 * 
 * Defines a skill and its initial level.
 */
USTRUCT(BlueprintType)
struct FFVSkillDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FGameplayTag SkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InitialLevel = 0.2f;
};

/**
 * UFVCharacterDataAsset
 * 
 * Data asset that defines a character's attributes, personality, skills, and initial effects.
 * Game designers create these in the editor and assign them to characters.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "FV Character Data", ShortTooltip = "Data asset defining character attributes and personality."))
class FLICKERVOIDCHARACTER_API UFVCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVCharacterDataAsset();

	// ========================================================================
	// IDENTIFICATION
	// ========================================================================
	
	/** Unique identifier for this character archetype */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName CharacterArchetypeId;

	/** Display name for the editor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Tags that describe this character (e.g., "Character.Type.Guard", "Character.Faction.Noble") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer CharacterTags;

	// ========================================================================
	// ATTRIBUTE INITIALIZATION
	// ========================================================================

	/** Initial attribute values - these override the AttributeSet defaults */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes", meta = (TitleProperty = "Attribute"))
	TArray<FFVAttributeInitializer> InitialAttributes;

	/** Skills this character possesses */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills", meta = (TitleProperty = "SkillTag"))
	TArray<FFVSkillDefinition> Skills;

	// ========================================================================
	// PERSONALITY PRESETS (Quick Setup)
	// ========================================================================

	/** Aggression preset for quick setup (Low/Medium/High) - overridden by InitialAttributes if specified */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Personality Presets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AggressionPreset = 0.5f;

	/** Courage preset for quick setup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Personality Presets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CouragePreset = 0.5f;

	/** Intelligence preset for quick setup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Personality Presets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IntelligencePreset = 0.5f;

	/** Corruption preset (moral flexibility) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Personality Presets", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CorruptionPreset = 0.0f;

	// ========================================================================
	// GAMEPLAY EFFECTS
	// ========================================================================

	/** Gameplay Effect used to initialize attributes from this data asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> AttributeInitializationEffect;

	/** Gameplay Effects to apply on spawn (e.g., passive effects, traits) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> InitialGameplayEffects;

	/** Trait tags to grant to this character (affects GAS tag queries) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FGameplayTagContainer GrantedTraitTags;

	// ========================================================================
	// BEHAVIOR CONFIGURATION
	// ========================================================================

	/** Initial state when spawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	FGameplayTag InitialStateTag;

	/** Threshold for transitioning to hostile state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HostilityThreshold = 0.7f;

	/** Threshold for transitioning to fleeing state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FleeThreshold = 0.3f;

	// ========================================================================
	// ASSET MANAGEMENT
	// ========================================================================

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};