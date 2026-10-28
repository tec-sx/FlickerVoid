#include "Items/FVItemDefinition.h"

#include "FVInventorySettings.h"
#include "FVInventoryTags.h"
#include "Items/FVItemFragments.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemDefinition)

#define LOCTEXT_NAMESPACE "FVItemDefinition"

bool UFVItemDefinition::IsEquippable() const
{
	return FindFragment<FFVItemFragment_Equippable>() != nullptr;
}

bool UFVItemDefinition::IsUsable() const
{
	return FindFragment<FFVItemFragment_Usable>() != nullptr;
}

FGameplayTagContainer UFVItemDefinition::GetEquipmentSlots() const
{
	const FFVItemFragment_Equippable* Equippable = FindFragment<FFVItemFragment_Equippable>();
	return Equippable ? Equippable->Slots : FGameplayTagContainer();
}

bool UFVItemDefinition::IsKeyItem() const
{
	return Category.IsValid() && Category.MatchesAny(UFVInventorySettings::Get().KeyItemCategories);
}

void UFVItemDefinition::PostLoad()
{
	Super::PostLoad();

	for (TInstancedStruct<FFVItemFragment>& Fragment : Fragments)
	{
		FFVItemFragment_Equippable* Equippable = Fragment.GetMutablePtr<FFVItemFragment_Equippable>();
		if (Equippable && Equippable->Slot_DEPRECATED.IsValid())
		{
			Equippable->Slots.AddTag(Equippable->Slot_DEPRECATED);
			Equippable->Slot_DEPRECATED = FGameplayTag();
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UFVItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (const FFVItemFragment_Equippable* Equippable = FindFragment<FFVItemFragment_Equippable>())
	{
		if (Equippable->Slots.IsEmpty())
		{
			Context.AddError(LOCTEXT("NoSlots", "Equippable has no Slots."));
			Result = EDataValidationResult::Invalid;
		}

		for (const FGameplayTag& Tag : Equippable->Slots)
		{
			if (!Tag.MatchesTag(FVInventoryTags::Equipment_Slot))
			{
				Context.AddError(FText::Format(LOCTEXT("NotASlot", "{0} is not under Equipment.Slot."), FText::FromName(Tag.GetTagName())));
				Result = EDataValidationResult::Invalid;
			}
		}

		if (Equippable->BlockedSlots.HasAnyExact(Equippable->Slots))
		{
			Context.AddError(LOCTEXT("BlocksOwnSlot", "Blocked Slots contains one of the item's own Slots."));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (FindFragment<FFVItemFragment_Container>() && !IsEquippable())
	{
		Context.AddWarning(LOCTEXT("ContainerNotEquippable", "Container adds space only while worn, but the item has no Equippable fragment."));
	}

	if (FindFragment<FFVItemFragment_Charges>() && !IsUsable())
	{
		Context.AddWarning(LOCTEXT("ChargesNotUsable", "Charges count uses, but the item has no Usable fragment."));
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
