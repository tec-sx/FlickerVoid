#include "FVNarrativeTags.h"

namespace FVNarrativeTags
{
	// ============================================================================
	// DIALOGUE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Speaker_Player, "Dialogue.Speaker.Player");

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
	// QUEST TYPE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_MainStory, "Quest.Type.MainStory", "Main storyline quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Side, "Quest.Type.Side", "Side quest.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Errand, "Quest.Type.Errand", "Simple errand task.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Quest_Type_Social, "Quest.Type.Social", "Social interaction quest.");

	// ============================================================================
	// FACT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Dialogue, "Fact.Dialogue",
								   "Root for dialogue facts, e.g. lines already spoken or topics unlocked.");
}
