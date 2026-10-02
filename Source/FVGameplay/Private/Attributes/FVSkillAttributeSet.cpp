#include "Attributes/FVSkillAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSkillAttributeSet)

void UFVSkillAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
Super::PreAttributeChange(Attribute, NewValue);
NewValue = FMath::Clamp(NewValue, MinSkill, MaxSkill);
}

void UFVSkillAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
Super::PreAttributeBaseChange(Attribute, NewValue);
NewValue = FMath::Clamp(NewValue, MinSkill, MaxSkill);
}