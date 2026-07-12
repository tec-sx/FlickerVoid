#include "FVAITags.h"

namespace FVAITags
{
	// ============================================================================
	// CHARACTER STATE TAGS (AI Behavior States)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Idle, "Character.State.Idle", "Character is idle, not engaged.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Alert, "Character.State.Alert", "Character is alert and watchful.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Suspicious, "Character.State.Suspicious", "Character is suspicious of something.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Hostile, "Character.State.Hostile", "Character is hostile.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Combat, "Character.State.Combat", "Character is in combat.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Fleeing, "Character.State.Fleeing", "Character is fleeing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Incapacitated, "Character.State.Incapacitated", "Character is incapacitated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Dead, "Character.State.Dead", "Character is dead.");

	// ============================================================================
	// NPC BEHAVIOR DECISION TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Ignore, "Behavior.Decision.Ignore", "NPC ignores the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Greet, "Behavior.Decision.Greet", "NPC greets the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Observe, "Behavior.Decision.Observe", "NPC observes the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_VerbalConfrontation, "Behavior.Decision.VerbalConfrontation", "NPC verbally confronts target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Threaten, "Behavior.Decision.Threaten", "NPC threatens the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Demand, "Behavior.Decision.Demand", "NPC demands something from target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Attack, "Behavior.Decision.Attack", "NPC attacks the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_CallForHelp, "Behavior.Decision.CallForHelp", "NPC calls for help.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Behavior_Decision_Flee, "Behavior.Decision.Flee", "NPC flees from target.");

	// ============================================================================
	// ATTACK STYLE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack_Style_BerserkerCharge, "Attack.Style.BerserkerCharge", "Aggressive rushing attack.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack_Style_CalculatedStrike, "Attack.Style.CalculatedStrike", "Careful, measured attack.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack_Style_RangedAmbush, "Attack.Style.RangedAmbush", "Ranged attack from distance.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack_Style_Grapple, "Attack.Style.Grapple", "Close-quarters grappling.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack_Style_CautiousProbe, "Attack.Style.CautiousProbe", "Cautious probing attack.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attack_Style_CowardlyBackstab, "Attack.Style.CowardlyBackstab", "Sneaky backstab attack.");

	// ============================================================================
	// PLAYER RESPONSE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Defend, "Response.Defend", "Player defends themselves.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Attack, "Response.Attack", "Player attacks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Intimidate, "Response.Intimidate", "Player intimidates target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Persuade, "Response.Persuade", "Player persuades target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Bribe, "Response.Bribe", "Player bribes target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Seduce, "Response.Seduce", "Player seduces target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Submit, "Response.Submit", "Player submits.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Threaten, "Response.Threaten", "Player threatens target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Response_Mock, "Response.Mock", "Player mocks target.");
	
	UE_DEFINE_GAMEPLAY_TAG(AI_Event_Idle, "AI.Event.Idle");
	UE_DEFINE_GAMEPLAY_TAG(AI_Event_Patrol, "AI.Event.Patrol");
	UE_DEFINE_GAMEPLAY_TAG(AI_Event_Notice, "AI.Event.Notice");
	UE_DEFINE_GAMEPLAY_TAG(AI_Event_Focus, "AI.Event.Focus");
	UE_DEFINE_GAMEPLAY_TAG(AI_Event_Act, "AI.Event.Act");

}
