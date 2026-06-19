// Fill out your copyright notice in the Description page of Project Settings.

#include "Attributes/FVCharacterAttributeComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Attributes/FVCharacterAttributeSet.h"
#include "Character/FVCharacterDataAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacterAttributeComponent)

UFVCharacterAttributeComponent::UFVCharacterAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CharacterData = nullptr;
}

void UFVCharacterAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheAbilitySystemComponent();

	if (CharacterData)
	{
		InitializeFromDataAsset();
	}
}

void UFVCharacterAttributeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove applied effects
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (const FActiveGameplayEffectHandle& Handle : AppliedEffectHandles)
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}
	AppliedEffectHandles.Empty();

	Super::EndPlay(EndPlayReason);
}

void UFVCharacterAttributeComponent::CacheAbilitySystemComponent()
{
	if (AActor* Owner = GetOwner())
	{
		CachedASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	}
}

UAbilitySystemComponent* UFVCharacterAttributeComponent::GetAbilitySystemComponent() const
{
	return CachedASC.Get();
}

const UFVCharacterAttributeSet* UFVCharacterAttributeComponent::GetAttributeSet() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->GetSet<UFVCharacterAttributeSet>();
	}
	return nullptr;
}

void UFVCharacterAttributeComponent::InitializeFromDataAsset()
{
	if (!CharacterData)
	{
		UE_LOG(LogTemp, Warning, TEXT("FVCharacterAttributeComponent: No CharacterData assigned to %s"), *GetOwner()->GetName());
		return;
	}

	InitializeFromDataAssetOverride(CharacterData);
}

void UFVCharacterAttributeComponent::InitializeFromDataAssetOverride(const UFVCharacterDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("FVCharacterAttributeComponent: No ASC found on %s"), *GetOwner()->GetName());
		return;
	}

	// Store skills
	for (const FFVSkillDefinition& Skill : DataAsset->Skills)
	{
		if (Skill.SkillTag.IsValid())
		{
			SkillLevels.Add(Skill.SkillTag, Skill.InitialLevel);
		}
	}

	// Apply attribute initialization effect
	ApplyAttributeInitialization();

	// Apply initial gameplay effects
	ApplyInitialGameplayEffects();

	// Grant trait tags
	GrantTraitTags();

	// Set initial state
	if (DataAsset->InitialStateTag.IsValid())
	{
		SetCharacterState(DataAsset->InitialStateTag);
	}
}

void UFVCharacterAttributeComponent::ApplyAttributeInitialization()
{
	if (!CharacterData)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply the initialization gameplay effect if specified
	if (CharacterData->AttributeInitializationEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			CharacterData->AttributeInitializationEffect, 
			1.0f, 
			ContextHandle
		);

		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (EffectHandle.IsValid())
			{
				AppliedEffectHandles.Add(EffectHandle);
			}
		}
	}

	// Apply individual attribute overrides from InitialAttributes array
	const UFVCharacterAttributeSet* AttributeSet = GetAttributeSet();
	if (!AttributeSet)
	{
		return;
	}

	for (const FFVAttributeInitializer& AttrInit : CharacterData->InitialAttributes)
	{
		if (AttrInit.Attribute.IsValid())
		{
			ASC->SetNumericAttributeBase(AttrInit.Attribute, AttrInit.Value);
		}
	}

	// Apply personality presets if no specific override was set
	auto SetPresetIfNotOverridden = [&](const FGameplayAttribute& Attr, float PresetValue)
	{
		bool bHasOverride = CharacterData->InitialAttributes.ContainsByPredicate(
			[&Attr](const FFVAttributeInitializer& Init) { return Init.Attribute == Attr; }
		);

		if (!bHasOverride)
		{
			ASC->SetNumericAttributeBase(Attr, PresetValue);
		}
	};

	SetPresetIfNotOverridden(UFVCharacterAttributeSet::GetAggressionAttribute(), CharacterData->AggressionPreset);
	SetPresetIfNotOverridden(UFVCharacterAttributeSet::GetCourageAttribute(), CharacterData->CouragePreset);
	SetPresetIfNotOverridden(UFVCharacterAttributeSet::GetIntelligenceAttribute(), CharacterData->IntelligencePreset);
	SetPresetIfNotOverridden(UFVCharacterAttributeSet::GetCorruptionAttribute(), CharacterData->CorruptionPreset);
}

void UFVCharacterAttributeComponent::ApplyInitialGameplayEffects()
{
	if (!CharacterData)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : CharacterData->InitialGameplayEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				if (EffectHandle.IsValid())
				{
					AppliedEffectHandles.Add(EffectHandle);
				}
			}
		}
	}
}

void UFVCharacterAttributeComponent::GrantTraitTags()
{
	if (!CharacterData)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Add trait tags as loose gameplay tags
	for (const FGameplayTag& Tag : CharacterData->GrantedTraitTags)
	{
		ASC->AddLooseGameplayTag(Tag);
	}

	// Add character identification tags
	for (const FGameplayTag& Tag : CharacterData->CharacterTags)
	{
		ASC->AddLooseGameplayTag(Tag);
	}
}

float UFVCharacterAttributeComponent::GetAttributeValue(FGameplayAttribute Attribute) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		bool bFound = false;
		float Value = ASC->GetGameplayAttributeValue(Attribute, bFound);
		return bFound ? Value : 0.0f;
	}
	return 0.0f;
}

float UFVCharacterAttributeComponent::GetAttributeValueByName(FName AttributeName) const
{
	const UFVCharacterAttributeSet* AttributeSet = GetAttributeSet();
	if (!AttributeSet)
	{
		return 0.0f;
	}

	// Find property by name
	const FProperty* Property = UFVCharacterAttributeSet::StaticClass()->FindPropertyByName(AttributeName);
	if (Property)
	{
		FGameplayAttribute Attribute(const_cast<FProperty*>(Property));
		return GetAttributeValue(Attribute);
	}

	return 0.0f;
}

void UFVCharacterAttributeComponent::SetCharacterState(FGameplayTag NewStateTag)
{
	if (!NewStateTag.IsValid() || NewStateTag == CurrentStateTag)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayTag OldState = CurrentStateTag;

	// Remove old state tag
	if (CurrentStateTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(CurrentStateTag);
	}

	// Add new state tag
	ASC->AddLooseGameplayTag(NewStateTag);
	CurrentStateTag = NewStateTag;

	OnCharacterStateChanged.Broadcast(OldState, NewStateTag);
}

bool UFVCharacterAttributeComponent::HasTag(FGameplayTag Tag) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	return false;
}

bool UFVCharacterAttributeComponent::HasAnyTag(const FGameplayTagContainer& Tags) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasAnyMatchingGameplayTags(Tags);
	}
	return false;
}

void UFVCharacterAttributeComponent::UpdateRelationship(AActor* Character, float Delta)
{
	if (!Character)
	{
		return;
	}

	float* Current = KnownCharacters.Find(Character);

	if (Current)
	{
		*Current = FMath::Clamp(*Current + Delta, -1.0f, 1.0f);
	}
	else
	{
		KnownCharacters.Add(Character, FMath::Clamp(Delta, -1.0f, 1.0f));
	}
}

float UFVCharacterAttributeComponent::GetRelationship(AActor* Character) const
{
	if (!Character)
	{
		return 0.0f;
	}

	const float* Relationship = KnownCharacters.Find(Character);
	return Relationship ? *Relationship : 0.0f;
}

float UFVCharacterAttributeComponent::GetSkillLevel(FGameplayTag SkillTag) const
{
	const float* Level = SkillLevels.Find(SkillTag);
	return Level ? *Level : 0.0f;
}

// ============================================================================
// DERIVED VALUES
// ============================================================================

float UFVCharacterAttributeComponent::GetPhysicalPower() const
{
	const UFVCharacterAttributeSet* Attrs = GetAttributeSet();
	if (!Attrs)
	{
		return 0.0f;
	}

	float Power = (Attrs->GetPhysique() * 0.4f) + 
				  (Attrs->GetStamina() * 0.3f) + 
				  (Attrs->GetHealth() * 0.3f);

	return FMath::Clamp(Power, 0.0f, 1.0f);
}

float UFVCharacterAttributeComponent::GetSocialPower() const
{
	const UFVCharacterAttributeSet* Attrs = GetAttributeSet();
	if (!Attrs)
	{
		return 0.0f;
	}

	return (Attrs->GetCharisma() * 0.3f) +
		   (Attrs->GetNotoriety() * 0.2f);
}

float UFVCharacterAttributeComponent::GetMentalStability() const
{
	const UFVCharacterAttributeSet* Attrs = GetAttributeSet();
	if (!Attrs)
	{
		return 0.0f;
	}

	float Stability = Attrs->GetMorale();
	Stability -= Attrs->GetStress() * 0.5f;
	Stability -= Attrs->GetRage() * 0.3f;
	Stability -= Attrs->GetFear() * 0.3f;
	Stability -= Attrs->GetIntoxication() * 0.4f;

	return FMath::Clamp(Stability, 0.0f, 1.0f);
}

float UFVCharacterAttributeComponent::GetCombatReadiness() const
{
	const UFVCharacterAttributeSet* Attrs = GetAttributeSet();
	if (!Attrs)
	{
		return 0.0f;
	}

	float Readiness = (Attrs->GetAlertness() * 0.3f) +
					  (Attrs->GetConfidence() * 0.2f) +
					  (GetPhysicalPower() * 0.3f) +
					  (GetMentalStability() * 0.2f);

	// Pain penalty check via GAS tags
	if (HasTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.Wounded"))))
	{
		Readiness *= FMath::Lerp(0.3f, 1.0f, Attrs->GetPainTolerance());
	}

	return FMath::Clamp(Readiness, 0.0f, 1.0f);
}

bool UFVCharacterAttributeComponent::HasEffect(FName EffectName) const
{
	// Check if we have a tag for this effect
	FGameplayTag EffectTag = FGameplayTag::RequestGameplayTag(EffectName, false);
	if (EffectTag.IsValid())
	{
		return HasTag(EffectTag);
	}
	return false;
}

float UFVCharacterAttributeComponent::GetModifiedAttributeValue(FName AttributeName) const
{
	return GetAttributeValueByName(AttributeName);
}

void UFVCharacterAttributeComponent::SetAttributeValue(FName AttributeName, float Value)
{
	const UFVCharacterAttributeSet* AttributeSet = GetAttributeSet();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	
	if (!AttributeSet || !ASC)
	{
		return;
	}

	// Find property by name
	const FProperty* Property = UFVCharacterAttributeSet::StaticClass()->FindPropertyByName(AttributeName);
	if (Property)
	{
		FGameplayAttribute Attribute(const_cast<FProperty*>(Property));
		ASC->SetNumericAttributeBase(Attribute, Value);
	}
}
