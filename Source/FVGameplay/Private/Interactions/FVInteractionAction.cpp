#pragma once

#include "Interactions/FVInteractionAction.h"

#include "UI/FVInteractionActionInfo.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionAction)

bool UFVInteractionAction::CheckRequirements(const FGameplayTagContainer& InstigatorTags) const
{
	bool bRequirementsMet = MatchAnyBlockedByTag 
		? InstigatorTags.HasAnyExact(BlockedByTags) 
		: InstigatorTags.HasAllExact(BlockedByTags);

	if (bRequirementsMet)
	{
		bRequirementsMet = MatchAnyRequiredTag 
		? InstigatorTags.HasAnyExact(RequiredTags) 
		: InstigatorTags.HasAllExact(RequiredTags);
	}

	return bRequirementsMet;
}

FFVInteractionActionInfo UFVInteractionAction::CreateActionUIInfo(const FGameplayTagContainer& InstigatorTags) const
{
	FFVInteractionActionInfo Entry;
	Entry.ActionTag   = ActionTag;
	Entry.Slot        = Slot;
	Entry.DisplayName = DisplayName;
	Entry.Icon        = Icon;
	Entry.bAvailable  = CheckRequirements(InstigatorTags);

	return Entry;
}
