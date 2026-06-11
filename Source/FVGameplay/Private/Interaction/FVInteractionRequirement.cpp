#include "Interaction/FVInteractionRequirement.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

//~=============================================================================
// UFVTagRequirement
//~=============================================================================

bool UFVTagRequirement::IsMet_Implementation(AActor* Instigator) const
{
	if (!Instigator || !RequiredTag.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(RequiredTag);
}

//~=============================================================================
// UFVAttributeRequirement
//~=============================================================================

bool UFVAttributeRequirement::IsMet_Implementation(AActor* Instigator) const
{
	if (!Instigator || !RequiredAttribute.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	if (!ASC)
	{
		return false;
	}

	bool bFound = false;
	const float Value = ASC->GetGameplayAttributeValue(RequiredAttribute, bFound);
	return bFound && Value >= MinimumValue;
}
