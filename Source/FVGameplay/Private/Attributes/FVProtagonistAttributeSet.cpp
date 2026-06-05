// Fill out your copyright notice in the Description page of Project Settings.

#include "Attributes/FVProtagonistAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVProtagonistAttributeSet)

UFVProtagonistAttributeSet::UFVProtagonistAttributeSet()
{
	// Sanity
	InitSanity(0.8f);
	InitMaxSanity(1.0f);
	InitSanityRecoveryRate(0.01f);
	InitSanityResistance(0.5f);

	// Addiction
	InitAlcoholDependency(0.0f);
	InitDrugDependency(0.0f);
	InitWithdrawalSeverity(0.0f);
	InitDaysClean(0.0f);

	// Memory
	InitMemoryFragmentsFound(0.0f);
	InitMemoryClarity(1.0f);

	// Meta
	InitIncomingSanityDamage(0.0f);
	InitIncomingSubstanceEffect(0.0f);
}

void UFVProtagonistAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Sanity clamped to 0 - MaxSanity
	if (Attribute == GetSanityAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSanity());
	}
	// Most attributes clamped 0-1
	else if (Attribute == GetMaxSanityAttribute() ||
			 Attribute == GetSanityRecoveryRateAttribute() ||
			 Attribute == GetSanityResistanceAttribute() ||
			 Attribute == GetAlcoholDependencyAttribute() ||
			 Attribute == GetDrugDependencyAttribute() ||
			 Attribute == GetWithdrawalSeverityAttribute() ||
			 Attribute == GetMemoryClarityAttribute())
	{
		ClampAttribute(NewValue, 0.0f, 1.0f);
	}
	// Days clean and memory fragments are non-negative
	else if (Attribute == GetDaysCleanAttribute() ||
			 Attribute == GetMemoryFragmentsFoundAttribute())
	{
		ClampAttribute(NewValue, 0.0f, FLT_MAX);
	}
}

void UFVProtagonistAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Process incoming sanity damage
	if (Data.EvaluatedData.Attribute == GetIncomingSanityDamageAttribute())
	{
		float LocalDamage = GetIncomingSanityDamage();
		SetIncomingSanityDamage(0.0f);

		if (LocalDamage > 0.0f)
		{
			// Apply resistance
			const float EffectiveDamage = LocalDamage * (1.0f - GetSanityResistance());
			const float NewSanity = GetSanity() - EffectiveDamage;
			SetSanity(FMath::Max(0.0f, NewSanity));

			UpdateSanityState();
		}
	}
	// Process substance effect
	else if (Data.EvaluatedData.Attribute == GetIncomingSubstanceEffectAttribute())
	{
		const float SubstanceEffect = GetIncomingSubstanceEffect();
		SetIncomingSubstanceEffect(0.0f);

		if (SubstanceEffect > 0.0f)
		{
			// Reset days clean counter when consuming substances
			SetDaysClean(0.0f);

			// Withdrawal is temporarily relieved by substance use
			if (GetWithdrawalSeverity() > 0.0f)
			{
				SetWithdrawalSeverity(FMath::Max(0.0f, GetWithdrawalSeverity() - SubstanceEffect));
			}
		}
	}
	// Update sanity state when sanity changes directly
	else if (Data.EvaluatedData.Attribute == GetSanityAttribute())
	{
		UpdateSanityState();
	}
}

void UFVProtagonistAttributeSet::ClampAttribute(float& NewValue, float MinValue, float MaxValue) const
{
	NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
}

void UFVProtagonistAttributeSet::UpdateSanityState()
{
	// This would typically update gameplay tags based on sanity thresholds
	// The actual tag management is handled by the owning component/subsystem
	// This method can be expanded to broadcast events
}

void UFVProtagonistAttributeSet::ProcessWithdrawal()
{
	// Calculate total dependency
	const float TotalDependency = GetAlcoholDependency() + GetDrugDependency();
	
	if (TotalDependency > 0.0f && GetDaysClean() > 0.0f)
	{
		// Withdrawal severity based on dependency level and time clean
		// Peaks around day 2-3, then gradually decreases
		const float DaysCleanClamped = FMath::Min(GetDaysClean(), 14.0f);
		const float WithdrawalCurve = FMath::Sin(DaysCleanClamped * PI / 7.0f) * (1.0f - DaysCleanClamped / 14.0f);
		const float NewWithdrawal = TotalDependency * WithdrawalCurve;
		
		SetWithdrawalSeverity(FMath::Max(0.0f, NewWithdrawal));
	}
}
