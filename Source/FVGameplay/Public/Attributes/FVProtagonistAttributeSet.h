// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "FVProtagonistAttributeSet.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UFVProtagonistAttributeSet
 * 
 * Attributes specific to the protagonist's psychological state.
 * Extends the base character attributes with sanity, addiction, and memory systems.
 */
UCLASS(MinimalAPI, BlueprintType)
class UFVProtagonistAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UFVProtagonistAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ========================================================================
	// SANITY SYSTEM
	// ========================================================================

	/** Current sanity level (0 = broken, 1 = perfectly stable) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Sanity")
	FGameplayAttributeData Sanity;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, Sanity)

	/** Maximum sanity capacity (can be reduced by trauma) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Sanity")
	FGameplayAttributeData MaxSanity;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, MaxSanity)

	/** Rate at which sanity naturally recovers (per game hour) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Sanity")
	FGameplayAttributeData SanityRecoveryRate;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, SanityRecoveryRate)

	/** Resistance to sanity loss (0-1, higher = more resistant) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Sanity")
	FGameplayAttributeData SanityResistance;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, SanityResistance)

	// ========================================================================
	// ADDICTION SYSTEM
	// ========================================================================

	/** Alcohol dependency level (0 = clean, 1 = fully addicted) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Addiction")
	FGameplayAttributeData AlcoholDependency;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, AlcoholDependency)

	/** Sedative dependency level */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Addiction")
	FGameplayAttributeData DrugDependency;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, DrugDependency)

	/** Current withdrawal severity (0 = none, 1 = severe) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Addiction")
	FGameplayAttributeData WithdrawalSeverity;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, WithdrawalSeverity)

	/** Days clean from substances (for recovery tracking) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Addiction")
	FGameplayAttributeData DaysClean;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, DaysClean)

	// ========================================================================
	// MEMORY & IDENTITY
	// ========================================================================

	/** Total memory fragments discovered */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Memory")
	FGameplayAttributeData MemoryFragmentsFound;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, MemoryFragmentsFound)

	/** Clarity of current memories (affected by substances/sanity) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Memory")
	FGameplayAttributeData MemoryClarity;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, MemoryClarity)

	// ========================================================================
	// META ATTRIBUTES
	// ========================================================================

	/** Incoming sanity damage (processed in PostGameplayEffectExecute) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingSanityDamage;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, IncomingSanityDamage)

	/** Incoming substance effect (processed for addiction) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingSubstanceEffect;
	ATTRIBUTE_ACCESSORS(UFVProtagonistAttributeSet, IncomingSubstanceEffect)

private:
	void ClampAttribute(float& NewValue, float MinValue, float MaxValue) const;
	void UpdateSanityState();
	void ProcessWithdrawal();
};

#undef ATTRIBUTE_ACCESSORS
#undef UE_API
