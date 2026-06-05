#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDWORLD_API

namespace FVWorldTags
{
	// ============================================================================
	// WORLD OBJECT TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldObject_Door);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldObject_Container);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldObject_Switch);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldObject_Junk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldObject_Decoration);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldObject_Furniture);
}

#undef UE_API
