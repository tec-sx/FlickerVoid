#include "FVNarrativeTags.h"

namespace FVNarrativeTags
{
	// ============================================================================
	// DIALOGUE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Speaker_Player, "Dialogue.Speaker.Player");
	
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_CallOut, "Dialogue.CallOut");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_LineReady, "Dialogue.LineReady");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_ChoicesReady, "Dialogue.ChoicesReady");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event, "Dialogue.Event");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Ended, "Dialogue.Ended");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_SubmitChoice, "Dialogue.SubmitChoice");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Continue, "Dialogue.Continue");
	
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Icon_Trade_Item, "Dialogue.Icon.Trade.Item");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Icon_Trade_Info, "Dialogue.Icon.Trade.Info");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Icon_Trade_Service, "Dialogue.Icon.Trade.Service");
	
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Icon_Question, "Dialogue.Icon.Question");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Icon_Flirt, "Dialogue.Icon.Flirt");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Icon_Threaten, "Dialogue.Icon.Threaten");
	
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_Agree, "Dialogue.Event.Agree");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_Offer, "Dialogue.Event.Offer");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_Refuse, "Dialogue.Event.Refuse");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_Flirt, "Dialogue.Event.Flirt");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_QuestAccepted, "Dialogue.Event.QuestAccepted");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_QuestCompleted, "Dialogue.Event.QuestCompleted");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event_QuestFailed, "Dialogue.Event.QuestFailed");
	
	// ============================================================================
	// QUEST STATE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Unavailable, "Quest.State.Unavailable", "Quest is not yet available.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Available, "Quest.State.Available", "Quest is available to start.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Active, "Quest.State.Active", "Quest is currently active.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Completed, "Quest.State.Completed", "Quest has been completed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Failed, "Quest.State.Failed", "Quest has been failed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_State_Abandoned, "Quest.State.Abandoned", "Quest was abandoned by player.");

	// ============================================================================
	// QUEST TYPE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_MainStory, "Quest.Type.MainStory", "Main storyline quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Side, "Quest.Type.Side", "Side quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Memory, "Quest.Type.Memory", "Memory recovery quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Rehabilitation, "Quest.Type.Rehabilitation", "Rehabilitation/recovery quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Errand, "Quest.Type.Errand", "Simple errand task.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Social, "Quest.Type.Social", "Social interaction quest.");

	// ============================================================================
	// OBJECTIVE TYPE TAGS
	// ============================================================================
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

	// ============================================================================
	// ADDICTION STATE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Clean, "Addiction.State.Clean", "No active addiction.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Casual, "Addiction.State.Casual", "Casual substance use.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Dependent, "Addiction.State.Dependent", "Substance dependent.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Addicted, "Addiction.State.Addicted", "Fully addicted.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Withdrawal, "Addiction.State.Withdrawal", "Experiencing withdrawal.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Addiction_State_Recovering, "Addiction.State.Recovering", "In recovery from addiction.");

	// ============================================================================
	// SUBSTANCE TYPE TAGS
	// ============================================================================
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
	// NARRATIVE PROGRESSION BLOCKER TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Sanity_TooLow, "Blocker.Sanity.TooLow", "Sanity too low to proceed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Addiction_Active, "Blocker.Addiction.Active", "Active addiction blocks progress.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Trust_Insufficient, "Blocker.Trust.Insufficient", "Insufficient NPC trust.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Memory_Required, "Blocker.Memory.Required", "Required memory not recovered.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocker_Rehabilitation_Required, "Blocker.Rehabilitation.Required", "Must complete rehabilitation.");
}
