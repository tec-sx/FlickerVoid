#include "FVScanDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScanDefinition)

const FFVScanCategory* UFVScannerSettings::FindCategory(const FGameplayTag& Category) const
{
	return Categories.FindByPredicate([&Category](const FFVScanCategory& It) { return Category.MatchesTag(It.Category); });
}
