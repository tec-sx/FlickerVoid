#include "Items/FVItemDefinition.h"

#include "Items/FVItemFragments.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemDefinition)

bool UFVItemDefinition::IsEquippable() const
{
	return FindFragment<FFVItemFragment_Equippable>() != nullptr;
}

bool UFVItemDefinition::IsUsable() const
{
	return FindFragment<FFVItemFragment_Usable>() != nullptr;
}

FGameplayTag UFVItemDefinition::GetEquipmentSlot() const
{
	const FFVItemFragment_Equippable* Equippable = FindFragment<FFVItemFragment_Equippable>();
	return Equippable ? Equippable->Slot : FGameplayTag();
}
