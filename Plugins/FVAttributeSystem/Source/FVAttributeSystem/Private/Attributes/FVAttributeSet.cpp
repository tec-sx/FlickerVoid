#include "Attributes/FVAttributeSet.h"

#include "Attributes/FVAttributeDefinition.h"
#include "Attributes/FVAttributeSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeSet)

void UFVAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampToDefinition(Attribute, NewValue);
}

void UFVAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampToDefinition(Attribute, NewValue);
}

void UFVAttributeSet::ClampToDefinition(const FGameplayAttribute& Attribute, float& Value) const
{
	if (const UFVAttributeSubsystem* Subsystem = UFVAttributeSubsystem::Get(GetOwningActor()))
	{
		if (const UFVAttributeDefinition* Definition = Subsystem->FindDefinition(Attribute))
		{
			Value = Definition->Clamp(Value);
		}
	}
}
