#include "Facts/FVFactSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactSettings)

const FFVFactDefinition* UFVFactSettings::FindDefinition(const FGameplayTag& Tag) const
{
	return Definitions.FindByPredicate([&Tag](const FFVFactDefinition& Definition)
	{
		return Definition.Tag == Tag;
	});
}

FName UFVFactSettings::GetValueName(const FGameplayTag& Tag, const int32 Value) const
{
	const FFVFactDefinition* Definition = FindDefinition(Tag);
	return Definition && Definition->ValueNames.IsValidIndex(Value) ? Definition->ValueNames[Value] : NAME_None;
}
