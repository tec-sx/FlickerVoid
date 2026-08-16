#pragma once

#include "Interactions/FVInteractionConfig.h"

#include "UI/FVInteractionInfo.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionConfig)

FFVInteractionInfo UFVInteractionConfig::CreateUIInfo() const
{
	FFVInteractionInfo Entry;
	Entry.AbilityTag  = AbilityTag;
	Entry.Slot        = Slot;
	Entry.DisplayName = DisplayName;
	Entry.Icon        = Icon;

	return Entry;
}
