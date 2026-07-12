#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDCHARACTER_API

namespace FVCharacterTags
{
	// ============================================================================
	// CHARACTER TRAIT TAGS (Permanent characteristics)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Aggressive);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Cowardly);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Greedy);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Honorable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Corrupt);

	// ============================================================================
	// CHARACTER MOOD TAGS (Temporary emotional states)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Happy);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Angry);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Fearful);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Intoxicated);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Stressed);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Horny);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Exhausted);
	
}

#undef UE_API
