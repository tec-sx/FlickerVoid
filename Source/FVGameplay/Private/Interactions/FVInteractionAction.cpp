#pragma once

#include "Interactions/FVInteractionAction.h"

#include "UI/FVInteractionActionInfo.h"

bool UFVInteractionAction::CheckRequirements(FGameplayTagContainer& InstigatorTags) const
{
	return InstigatorTags.HasAll(RequiredTags);
}

FFVInteractionActionInfo UFVInteractionAction::CreateActionUIInfo(FGameplayTagContainer& InstigatorTags) const
{
	FFVInteractionActionInfo Entry;
	Entry.ActionTag   = ActionTag;
	Entry.InputTag    = InputTag;
	Entry.DisplayName = DisplayName;
	Entry.Icon        = Icon;

	FText UnmetReason;
	Entry.bAvailable        = CheckRequirements(InstigatorTags);
	Entry.UnavailableReason = UnmetReason;
	
	return Entry;
}
