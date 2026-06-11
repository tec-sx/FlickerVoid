#include "FVCoreTags.h"

namespace FVCoreTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Walk, "InputTag.Walk", "Walk input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sprint, "InputTag.Sprint", "Sprint input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "Jump input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Aim, "InputTag.Aim", "Aim input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact_Primary,       "InputTag.Interact.Primary",       "Interaction slot 1 — e.g. E press.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact_PrimaryHold,   "InputTag.Interact.PrimaryHold",   "Interaction slot 2 — e.g. E hold.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact_Secondary,     "InputTag.Interact.Secondary",     "Interaction slot 3 — e.g. F press.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact_SecondaryHold, "InputTag.Interact.SecondaryHold", "Interaction slot 4 — e.g. F hold.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Jogging, "Status.Jogging", "Target is Jogging.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Sprinting, "Status.Sprinting", "Target is Sprinting.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");

	// ============================================================================
	// INTERACTION ACTION TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_ExamineStarted,  "Interaction.Event.ExamineStarted",  "Fired when an examine overlay should open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_LockpickStarted, "Interaction.Event.LockpickStarted", "Fired when a lockpick mini-game should open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_LockpickEnded,   "Interaction.Event.LockpickEnded",   "Fired when the lockpick mini-game closes.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Pickup, "Interaction.Action.Pickup", "Pick up an item from the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Use, "Interaction.Action.Use", "Use/activate an interactable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Open, "Interaction.Action.Open", "Open a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Close, "Interaction.Action.Close", "Close a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Talk, "Interaction.Action.Talk", "Talk to an NPC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Examine, "Interaction.Action.Examine", "Examine/inspect an object.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Read, "Interaction.Action.Read", "Read a document/note.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Activate, "Interaction.Action.Activate", "Activate a device/switch.");

	// ============================================================================
	// INTERACTABLE TYPE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item, "Interactable.Item", "An item that can be interacted with.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item_Pickup, "Interactable.Item.Pickup", "An item pickup in the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Door, "Interactable.Door", "A door that can be opened/closed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Container, "Interactable.Container", "A container with items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_NPC, "Interactable.NPC", "An NPC that can be talked to.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Device, "Interactable.Device", "A device that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Document, "Interactable.Document", "A readable document.");


	// ============================================================================
	// CHARACTER REQUIREMENT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Class_Protagonist, "Character.Class.Protagonist", "The protagonist class.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Level_1, "Character.Level.1", "Character level 1.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Level_10, "Character.Level.10", "Character level 10.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Level_20, "Character.Level.20", "Character level 20.");

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

	// ============================================================================
	// CHARACTER MOOD TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Happy, "Character.Mood.Happy", "Character is happy.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Angry, "Character.Mood.Angry", "Character is angry.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Fearful, "Character.Mood.Fearful", "Character is fearful.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Intoxicated, "Character.Mood.Intoxicated", "Character is intoxicated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Stressed, "Character.Mood.Stressed", "Character is stressed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Horny, "Character.Mood.Horny", "Character is aroused.");

	// ============================================================================
	// CHARACTER TRAIT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Aggressive, "Character.Trait.Aggressive", "Character has aggressive personality.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Cowardly, "Character.Trait.Cowardly", "Character has cowardly personality.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Greedy, "Character.Trait.Greedy", "Character is greedy.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Honorable, "Character.Trait.Honorable", "Character is honorable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Corrupt, "Character.Trait.Corrupt", "Character is corrupt.");

	// ============================================================================
	// NARRATIVE & QUEST TAGS
	// ============================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Unavailable, "Quest.State.Unavailable", "Quest is not yet available.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Available, "Quest.State.Available", "Quest is available to start.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Active, "Quest.State.Active", "Quest is currently active.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Completed, "Quest.State.Completed", "Quest has been completed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Failed, "Quest.State.Failed", "Quest has been failed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Abandoned, "Quest.State.Abandoned", "Quest was abandoned by player.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_MainStory, "Quest.Type.MainStory", "Main storyline quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Side, "Quest.Type.Side", "Side quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Memory, "Quest.Type.Memory", "Memory recovery quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Rehabilitation, "Quest.Type.Rehabilitation", "Rehabilitation/recovery quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Errand, "Quest.Type.Errand", "Simple errand task.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Social, "Quest.Type.Social", "Social interaction quest.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Talk, "Objective.Type.Talk", "Talk to an NPC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Investigate, "Objective.Type.Investigate", "Investigate something.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Infiltrate, "Objective.Type.Infiltrate", "Infiltrate a location.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Obtain, "Objective.Type.Obtain", "Obtain an item.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Deliver, "Objective.Type.Deliver", "Deliver an item.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Attend, "Objective.Type.Attend", "Attend an event.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Wait, "Objective.Type.Wait", "Wait for time to pass.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Objective_Type_Threshold, "Objective.Type.Threshold", "Reach a threshold.");
	UE_DEFINE_GAMEPLAY_TAG(Objective_Type_MaintainState, "Objective.Type.MaintainState");
	UE_DEFINE_GAMEPLAY_TAG(Objective_Type_Location, "Objective.Type.Location");

	// ============================================================================
	// SANITY & PSYCHOLOGICAL STATE TAGS
	// ============================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sanity_State_Stable, "Sanity.State.Stable", "Protagonist is mentally stable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sanity_State_Anxious, "Sanity.State.Anxious", "Protagonist is anxious.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sanity_State_Disturbed, "Sanity.State.Disturbed", "Protagonist is disturbed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sanity_State_Unstable, "Sanity.State.Unstable", "Protagonist is mentally unstable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sanity_State_Breaking, "Sanity.State.Breaking", "Protagonist is on the verge of breakdown.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sanity_State_Broken, "Sanity.State.Broken", "Protagonist has had a mental breakdown.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Clean, "Addiction.State.Clean", "No active addiction.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Casual, "Addiction.State.Casual", "Casual substance use.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Dependent, "Addiction.State.Dependent", "Substance dependent.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Addicted, "Addiction.State.Addicted", "Fully addicted.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Withdrawal, "Addiction.State.Withdrawal", "Experiencing withdrawal.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Recovering, "Addiction.State.Recovering", "In recovery from addiction.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Substance_Alcohol, "Substance.Alcohol", "Alcohol substance.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Substance_Sedatives, "Substance.Sedatives", "Sedative substances.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Substance_Stimulants, "Substance.Stimulants", "Stimulant substances.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Substance_Hallucinogens, "Substance.Hallucinogens", "Hallucinogenic substances.");

	// ============================================================================
	// MEMORY & IDENTITY TAGS
	// ============================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Type_Identity, "Memory.Type.Identity", "Memory about self-identity.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Type_Relationship, "Memory.Type.Relationship", "Memory about relationships.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Type_Skill, "Memory.Type.Skill", "Memory unlocking a skill.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Type_Trauma, "Memory.Type.Trauma", "Traumatic memory.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Type_Location, "Memory.Type.Location", "Memory about a location.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Clarity_Fragment, "Memory.Clarity.Fragment", "Fragmentary, unclear memory.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Clarity_Partial, "Memory.Clarity.Partial", "Partially clear memory.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Memory_Clarity_Clear, "Memory.Clarity.Clear", "Fully clear memory.");

	// ============================================================================
	// WORLD STATE & CONSEQUENCE TAGS
	// ============================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldState_Chapter_1, "WorldState.Chapter.1", "Story is in Chapter 1.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldState_Chapter_2, "WorldState.Chapter.2", "Story is in Chapter 2.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldState_Chapter_3, "WorldState.Chapter.3", "Story is in Chapter 3.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consequence_Reputation_Gain, "Consequence.Reputation.Gain", "Player gained reputation.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consequence_Reputation_Loss, "Consequence.Reputation.Loss", "Player lost reputation.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consequence_Trust_Gain, "Consequence.Trust.Gain", "Player gained trust with NPC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consequence_Trust_Loss, "Consequence.Trust.Loss", "Player lost trust with NPC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consequence_Memory_Unlock, "Consequence.Memory.Unlock", "Memory was unlocked.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Consequence_Sanity_Impact, "Consequence.Sanity.Impact", "Sanity was affected.");

	// ============================================================================
	// NARRATIVE PROGRESSION BLOCKERS
	// ============================================================================

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Sanity_TooLow, "Blocker.Sanity.TooLow", "Sanity too low to proceed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Addiction_Active, "Blocker.Addiction.Active", "Active addiction blocks progress.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Trust_Insufficient, "Blocker.Trust.Insufficient", "Insufficient NPC trust.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Memory_Required, "Blocker.Memory.Required", "Required memory not recovered.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Rehabilitation_Required, "Blocker.Rehabilitation.Required", "Must complete rehabilitation.");

	// ============================================================================
	// EFFECT TAGS
	// ============================================================================

	UE_DEFINE_GAMEPLAY_TAG(Effect_Equipment, "Effect.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable, "Effect.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Food, "Effect.Consumable.Food");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Drug, "Effect.Consumable.Drug");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Beverage, "Effect.Consumable.Beverage");

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}