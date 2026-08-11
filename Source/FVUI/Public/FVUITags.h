#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDUI_API

namespace FVUITags
{
	// ============================================================================
	// MESSAGE CHANNELS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Inventory_Changed);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_LineReady);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_ChoicesReady);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Event);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Ended);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_SubmitChoice);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dialogue_Continue);
}

#undef UE_API
