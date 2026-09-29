#include "FVCharacterTags.h"

namespace FVCharacterTags
{
	// ============================================================================
	// CHARACTER TRAIT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Aggressive, "Character.Trait.Aggressive", "Aggressive trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Cowardly, "Character.Trait.Cowardly", "Cowardly trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Greedy, "Character.Trait.Greedy", "Greedy trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Honorable, "Character.Trait.Honorable", "Honorable trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Corrupt, "Character.Trait.Corrupt", " Corrupt trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Cautious, "Character.Trait.Cautious", "Cautious trait.");

	// ============================================================================
	// CHARACTER MOOD TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Happy, "Character.Mood.Happy", "Happy mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Angry, "Character.Mood.Angry", "Angry mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Fearful, "Character.Mood.Fearful", "Fearful mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Intoxicated, "Character.Mood.Intoxicated", "Intoxicated mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Stressed, "Character.Mood.Stressed", "Stressed mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Horny, "Character.Mood.Horny", "Aroused mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Exhausted, "Character.Mood.Exhausted", "Exhausted mood.");

	// ============================================================================
	// FACT TAGS (PLAYER)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Player, "Fact.Player", "Root for facts about the player character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Player_Knows, "Fact.Player.Knows",
								   "Information the player has learned. Gates dialogue options.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Player_Choice, "Fact.Player.Choice",
								   "Irreversible choices the player has made.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Player_Skill, "Fact.Player.Skill", "Skills or perks the player has acquired.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Player_State, "Fact.Player.State",
								   "Persistent narrative state of the player, e.g. sanity or addiction tiers.");

}
