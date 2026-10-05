#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDNARRATIVE_API

namespace FVNarrativeTags
{
	// ============================================================================
	// DIALOGUE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Speaker_Player);

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
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Errand);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Social);

	// Fact.Dialogue root for dialogue facts (lines spoken, topics unlocked).
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Dialogue);
}

#undef UE_API
