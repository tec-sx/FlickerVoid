#include "Relevance/FVRelevanceSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRelevanceSettings)

EFVRelevanceBand FFVRelevanceGroup::ComputeBand(const float Distance, const EFVRelevanceBand CurrentBand) const
{
	// A band is only left once the distance passes its outer threshold plus the hysteresis.
	const float NearExit = NearDistance + (CurrentBand == EFVRelevanceBand::Near ? Hysteresis : 0.f);
	const float MidExit = MidDistance + (CurrentBand != EFVRelevanceBand::Far ? Hysteresis : 0.f);

	if (Distance <= NearExit)
	{
		return EFVRelevanceBand::Near;
	}
	return Distance <= MidExit ? EFVRelevanceBand::Mid : EFVRelevanceBand::Far;
}

const FFVRelevanceGroup* UFVRelevanceSettings::FindGroup(const FGameplayTag& Group) const
{
	return Groups.FindByPredicate([&Group](const FFVRelevanceGroup& It) { return It.Group == Group; });
}
