#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

namespace FVGameplayTags
{
	// ============================================================================
	// ABILITY ACTIVATION FAILURE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

	// ============================================================================
	// EFFECT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Equipment);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Food);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Drug);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Beverage);
}

#undef UE_API
