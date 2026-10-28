#include "Attributes/FVAttributeLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Attributes/FVAttributeDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeLibrary)

UAbilitySystemComponent* UFVAttributeLibrary::FindAbilitySystem(const AActor* Actor, const UFVAttributeDefinition* Attribute)
{
	if (Actor == nullptr || Attribute == nullptr || !Attribute->Attribute.IsValid())
	{
		return nullptr;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	return AbilitySystem != nullptr && AbilitySystem->HasAttributeSetForAttribute(Attribute->Attribute) ? AbilitySystem : nullptr;
}

bool UFVAttributeLibrary::HasAttribute(const AActor* Actor, const UFVAttributeDefinition* Attribute)
{
	return FindAbilitySystem(Actor, Attribute) != nullptr;
}

float UFVAttributeLibrary::GetValue(const AActor* Actor, const UFVAttributeDefinition* Attribute)
{
	const UAbilitySystemComponent* AbilitySystem = FindAbilitySystem(Actor, Attribute);
	return AbilitySystem != nullptr ? AbilitySystem->GetNumericAttribute(Attribute->Attribute) : 0.f;
}

float UFVAttributeLibrary::GetBaseValue(const AActor* Actor, const UFVAttributeDefinition* Attribute)
{
	const UAbilitySystemComponent* AbilitySystem = FindAbilitySystem(Actor, Attribute);
	return AbilitySystem != nullptr ? AbilitySystem->GetNumericAttributeBase(Attribute->Attribute) : 0.f;
}

void UFVAttributeLibrary::SetBaseValue(AActor* Actor, const UFVAttributeDefinition* Attribute, float Value)
{
	if (UAbilitySystemComponent* AbilitySystem = FindAbilitySystem(Actor, Attribute))
	{
		AbilitySystem->SetNumericAttributeBase(Attribute->Attribute, Value);
	}
}

void UFVAttributeLibrary::AddToBaseValue(AActor* Actor, const UFVAttributeDefinition* Attribute, float Delta)
{
	if (UAbilitySystemComponent* AbilitySystem = FindAbilitySystem(Actor, Attribute))
	{
		AbilitySystem->SetNumericAttributeBase(Attribute->Attribute, AbilitySystem->GetNumericAttributeBase(Attribute->Attribute) + Delta);
	}
}
