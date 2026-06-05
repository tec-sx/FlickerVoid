// Fill out your copyright notice in the Description page of Project Settings.

#include "Attributes/FVCharacterAttributeSet.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacterAttributeSet)

UFVCharacterAttributeSet::UFVCharacterAttributeSet()
{
	// Social
	InitReputation(0.0f);
	InitNotoriety(0.0f);
	InitCharisma(0.5f);
	
	// Personality
	InitAggression(0.5f);
	InitLibido(0.5f);
	InitIntelligence(0.5f);
	InitCorruption(0.0f);
	InitCourage(0.5f);
	InitEmpathy(0.5f);
	InitGreed(0.3f);
	
	// Mood
	InitMorale(0.7f);
	InitStress(0.2f);
	InitRage(0.0f);
	InitFear(0.0f);
	InitConfidence(0.5f);
	InitAlertness(0.5f);
	InitIntoxication(0.0f);
	
	// Physical
	InitHealth(1.0f);
	InitStamina(1.0f);
	InitPhysique(0.5f);
	InitPainTolerance(0.5f);
}

void UFVCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp Reputation to -1 to 1, all others to 0 to 1
	if (Attribute == GetReputationAttribute())
	{
		ClampAttribute(Attribute, NewValue, -1.0f, 1.0f);
	}
	else
	{
		ClampAttribute(Attribute, NewValue, 0.0f, 1.0f);
	}
}

void UFVCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Handle meta attribute: IncomingDamage
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (LocalDamage > 0.0f)
		{
			const float NewHealth = GetHealth() - LocalDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, 1.0f));
		}
	}
	// Handle meta attribute: IncomingPersuasion
	else if (Data.EvaluatedData.Attribute == GetIncomingPersuasionAttribute())
	{
		const float PersuasionPower = GetIncomingPersuasion();
		SetIncomingPersuasion(0.0f);

		if (PersuasionPower != 0.0f)
		{
			// Empathetic characters are more receptive to persuasion
			const float Receptiveness = GetEmpathy() * 0.5f;
			const float MoraleChange = PersuasionPower * Receptiveness;
			SetMorale(FMath::Clamp(GetMorale() + MoraleChange, 0.0f, 1.0f));
		}
	}
}

void UFVCharacterAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue, float MinValue, float MaxValue) const
{
	NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
}
