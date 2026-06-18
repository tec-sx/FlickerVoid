#pragma once

#include "Interactions/FVInteractionAction.h"

#include "UI/FVInteractionActionInfo.h"

bool UFVInteractionAction::CheckRequirements(FGameplayTagContainer& InstigatorTags) const
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

FFVInteractionActionInfo UFVInteractionAction::CreateActionUIInfo(FGameplayTagContainer& InstigatorTags) const
{
	FFVInteractionActionInfo Entry;
	Entry.ActionTag   = ActionTag;
	Entry.DisplayName = DisplayName;
	Entry.Icon        = Icon;
	FText UnmetReason;
	Entry.bAvailable        = CheckRequirements(InstigatorTags);
	Entry.UnavailableReason = UnmetReason;
	
	return Entry;
}
