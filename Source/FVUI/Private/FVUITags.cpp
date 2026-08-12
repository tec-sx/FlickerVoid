#include "FVUITags.h"

namespace FVUITags
{
	// ============================================================================
	// MESSAGE CHANNELS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(UI_Inventory_Changed, "UI.Inventory.Changed");
	UE_DEFINE_GAMEPLAY_TAG(UI_Interaction_PromptChanged, "UI.Interaction.PromptChanged");

	UE_DEFINE_GAMEPLAY_TAG(Dialogue_LineReady, "Dialogue.LineReady");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_ChoicesReady, "Dialogue.ChoicesReady");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Event, "Dialogue.Event");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Ended, "Dialogue.Ended");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_SubmitChoice, "Dialogue.SubmitChoice");
	UE_DEFINE_GAMEPLAY_TAG(Dialogue_Continue, "Dialogue.Continue");
}
