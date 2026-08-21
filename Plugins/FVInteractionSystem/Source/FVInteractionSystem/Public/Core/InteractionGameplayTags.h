#pragma once

#include "NativeGameplayTags.h"

#define UE_API FVINTERACTIONSYSTEM_API

namespace InteractionTags
{
	/** Root of the action tags an interactable can offer. */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action);

	/** Root of the input tags an interaction offer can be keyed by. */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction);
}

#undef UE_API
