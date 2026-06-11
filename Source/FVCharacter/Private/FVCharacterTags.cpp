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
	// CHARACTER STATE TAGS (AI Behavior States)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Idle, "Character.State.Idle", "Character is idle, not engaged.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Alert, "Character.State.Alert", "Character is alert and watchful.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Suspicious, "Character.State.Suspicious", "Character is suspicious of something.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Hostile, "Character.State.Hostile", "Character is hostile.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Fleeing, "Character.State.Fleeing", "Character is fleeing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Incapacitated, "Character.State.Incapacitated", "Character is incapacitated.");
}
