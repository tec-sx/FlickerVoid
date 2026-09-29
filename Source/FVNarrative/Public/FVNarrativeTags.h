#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDNARRATIVE_API

namespace FVNarrativeTags
{
	// ============================================================================
	// DIALOGUE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Speaker_Player);
	
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_CallOut);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_LineReady);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_ChoicesReady);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Ended);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_SubmitChoice);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Continue);
	
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Icon_Trade_Item);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Icon_Trade_Info);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Icon_Trade_Service);
	
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Icon_Question);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Icon_Flirt);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Icon_Threaten);
	
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_Agree);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_Offer);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_Refuse);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_Flirt);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_QuestAccepted);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_QuestCompleted);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event_QuestFailed);

	// ============================================================================
	// QUEST TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_MainStory);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Side);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Memory);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Rehabilitation);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Errand);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Social);

	// ============================================================================
	// SANITY & PSYCHOLOGICAL STATE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Stable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Anxious);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Disturbed);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Unstable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Breaking);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Broken);

	// ============================================================================
	// ADDICTION STATE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Clean);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Casual);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Dependent);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Addicted);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Withdrawal);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Recovering);

	// ============================================================================
	// SUBSTANCE TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Alcohol);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Sedatives);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Stimulants);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Hallucinogens);

	// ============================================================================
	// MEMORY & IDENTITY TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Identity);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Relationship);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Skill);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Trauma);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Location);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Clarity_Fragment);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Clarity_Partial);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Clarity_Clear);

	// ============================================================================
	// WORLD STATE & CONSEQUENCE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldState_Chapter_1);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldState_Chapter_2);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldState_Chapter_3);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Reputation_Gain);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Reputation_Loss);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Trust_Gain);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Trust_Loss);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Memory_Unlock);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Sanity_Impact);

	// ============================================================================
	// NARRATIVE PROGRESSION BLOCKER TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Sanity_TooLow);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Addiction_Active);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Trust_Insufficient);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Memory_Required);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Rehabilitation_Required);

	// ============================================================================
	// FACT TAGS (CHAPTER / QUEST SYSTEM)
	// Roots only. Concrete facts are composed at runtime by FVQuestFactHelpers:
	//   Fact.Chapter.<ChapterId>.Stage                            - sparse EFVQuestStage value
	//   Fact.Chapter.<ChapterId>.Outcome                          - designer defined outcome id
	//   Fact.Quest.<ChapterId>.<QuestId>.Stage                    - sparse EFVQuestStage value
	//   Fact.Quest.<ChapterId>.<QuestId>.Outcome                  - designer defined outcome id
	//   Fact.Quest.<ChapterId>.<QuestId>.Objective.<ObjId>        - undefined not issued, 0 active, 1 done
	//   Fact.Quest.<ChapterId>.<QuestId>.Counter.<Counter>        - per quest tallies
	// Quests nest under their chapter so UndefineFactsUnderTag can wipe either a whole
	// chapter (Fact.Quest.<ChapterId>) or a single quest (Fact.Quest.<ChapterId>.<QuestId>).
	//
	// Whether a quest is REQUIRED to progress its chapter is deliberately not a fact.
	// That is authored, immutable data; the fact DB holds mutable runtime state. The
	// chapter's Flow graph declares it, by naming the gating quests in the condition
	// group of its Wait For Fact Condition node.
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Chapter);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Chapter_Current);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Quest);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Dialogue);
}

#undef UE_API
