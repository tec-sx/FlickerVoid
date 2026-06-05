#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDNARRATIVE_API

namespace FVNarrativeTags
{
	// ============================================================================
	// QUEST STATE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Unavailable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Available);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Active);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Completed);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Failed);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Abandoned);

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
	// OBJECTIVE TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Talk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Investigate);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Infiltrate);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Obtain);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Deliver);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Attend);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Wait);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Threshold);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_MaintainState);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Location);

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
}

#undef UE_API
