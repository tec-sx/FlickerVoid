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

	// ============================================================================
	// FACT TAGS (WORLD)
	// Locations follow Fact.Location.<LocationId>.<Aspect>, e.g.
	//   Fact.Location.Docks.Discovered
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_World);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_World_Time);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_World_Time_Day);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_World_Time_Hour);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_World_Time_Phase);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_World_Flag);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Location);
}

#undef UE_API
