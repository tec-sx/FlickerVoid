#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "FVSkillAttributeSet.generated.h"

#define SKILL_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Skill ratings used by UFVCheckDefinition. */
UCLASS(BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVSkillAttributeSet : public UAttributeSet
{
GENERATED_BODY()

public:
static constexpr float MinSkill = 0.f;
static constexpr float MaxSkill = 20.f;

virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Perception;
SKILL_ACCESSORS(UFVSkillAttributeSet, Perception)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Logic;
SKILL_ACCESSORS(UFVSkillAttributeSet, Logic)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Rhetoric;
SKILL_ACCESSORS(UFVSkillAttributeSet, Rhetoric)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Intimidation;
SKILL_ACCESSORS(UFVSkillAttributeSet, Intimidation)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Composure;
SKILL_ACCESSORS(UFVSkillAttributeSet, Composure)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Streetwise;
SKILL_ACCESSORS(UFVSkillAttributeSet, Streetwise)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Endurance;
SKILL_ACCESSORS(UFVSkillAttributeSet, Endurance)

UPROPERTY(BlueprintReadOnly, Category = "Attributes|Skills")
FGameplayAttributeData Reflexes;
SKILL_ACCESSORS(UFVSkillAttributeSet, Reflexes)
};

#undef SKILL_ACCESSORS