#pragma once

#include "Interactions/FVInteractionAction.h"

#include "UI/FVInteractionActionInfo.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionAction)

FFVInteractionActionInfo UFVInteractionAction::CreateActionUIInfo() const
{
	FFVInteractionActionInfo Entry;
	Entry.ActionTag   = AbilityTag;
	Entry.Slot        = Slot;
	Entry.DisplayName = DisplayName;
	Entry.Icon        = Icon;

	return Entry;
}
