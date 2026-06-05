// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "FVCharacterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UFVCharacterAttributeSet
 * 
 * GAS-native attribute set for character attributes.
 * All attributes are 0.0-1.0 normalized (except Reputation which is -1.0 to 1.0).
 * Supports Gameplay Effects for modification.
 */
UCLASS(MinimalAPI, BlueprintType)
class UFVCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UFVCharacterAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ========================================================================
	// SOCIAL ATTRIBUTES
	// ========================================================================
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Social")
	FGameplayAttributeData Reputation; // -1 to 1
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Reputation)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Social")
	FGameplayAttributeData Notoriety;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Notoriety)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Social")
	FGameplayAttributeData Charisma;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Charisma)

	// ========================================================================
	// PERSONALITY TRAITS
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Aggression;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Aggression)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Libido;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Libido)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Intelligence)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Corruption;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Corruption)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Courage;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Courage)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Empathy;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Empathy)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Personality")
	FGameplayAttributeData Greed;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Greed)

	// ========================================================================
	// MOOD / MENTAL STATE
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Morale;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Morale)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Stress;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Stress)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Rage;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Rage)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Fear;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Fear)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Confidence;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Confidence)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Alertness;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Alertness)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mood")
	FGameplayAttributeData Intoxication;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Intoxication)

	// ========================================================================
	// PHYSICAL - BODY
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Physical")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Physical")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Physical")
	FGameplayAttributeData Physique;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, Physique)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Physical")
	FGameplayAttributeData PainTolerance;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, PainTolerance)

	// ========================================================================
	// META ATTRIBUTES (used by Gameplay Effects, not replicated)
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingPersuasion;
	ATTRIBUTE_ACCESSORS(UFVCharacterAttributeSet, IncomingPersuasion)

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue, float MinValue, float MaxValue) const;
};

#undef ATTRIBUTE_ACCESSORS
