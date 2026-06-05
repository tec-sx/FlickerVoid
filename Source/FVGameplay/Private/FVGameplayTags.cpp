#include "FVGameplayTags.h"

namespace FVGameplayTags
{
	// ============================================================================
	// ABILITY ACTIVATION FAILURE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	// ============================================================================
	// EFFECT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Effect_Equipment, "Effect.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable, "Effect.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Food, "Effect.Consumable.Food");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Drug, "Effect.Consumable.Drug");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Beverage, "Effect.Consumable.Beverage");
}
