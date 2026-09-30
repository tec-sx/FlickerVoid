#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDUI_API

namespace FVUITags
{
	// ============================================================================
	// MESSAGE CHANNELS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Interaction_PromptChanged);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);

}

#undef UE_API
