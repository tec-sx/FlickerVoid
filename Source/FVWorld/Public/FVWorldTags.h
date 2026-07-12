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
	
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_TimeOfDay_Morning);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_TimeOfDay_Afternoon);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_TimeOfDay_Evening);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World_TimeOfDay_Night);
}

#undef UE_API
