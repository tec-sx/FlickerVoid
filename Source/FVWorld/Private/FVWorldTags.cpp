#include "FVWorldTags.h"

namespace FVWorldTags
{
	// ============================================================================
	// WORLD OBJECT TYPE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldObject_Door, "WorldObject.Door", "A door that can be opened/closed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldObject_Container, "WorldObject.Container", "A container that holds items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldObject_Switch, "WorldObject.Switch", "A switch or button that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldObject_Junk, "WorldObject.Junk", "Junk or debris, character may refuse to interact.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldObject_Decoration, "WorldObject.Decoration", "Decorative object, can be examined.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(WorldObject_Furniture, "WorldObject.Furniture", "Furniture like chairs, tables, etc.");
	
	UE_DEFINE_GAMEPLAY_TAG(World_TimeOfDay_Morning, "World.TimeOfDay.Morning");
	UE_DEFINE_GAMEPLAY_TAG(World_TimeOfDay_Afternoon, "World.TimeOfDay.Afternoon");
	UE_DEFINE_GAMEPLAY_TAG(World_TimeOfDay_Evening, "World.TimeOfDay.Evening");
	UE_DEFINE_GAMEPLAY_TAG(World_TimeOfDay_Night, "World.TimeOfDay.Night");

	// ============================================================================
	// FACT TAGS (WORLD)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_World, "Fact.World", "Root for global world state facts.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_World_Time, "Fact.World.Time",
								   "Time related facts such as elapsed days or current time of day index.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_World_Time_Day, "Fact.World.Time.Day",
								   "Elapsed whole days since game start, starting at 0. Mirrors the Day Sequence clock.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_World_Time_Hour, "Fact.World.Time.Hour",
								   "Local hour of the current day, 0-23. Mirrors the Day Sequence clock.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_World_Time_Phase, "Fact.World.Time.Phase",
								   "Coarse time of day as an EFVTimeOfDay value. Mirrors the Day Sequence clock.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_World_Flag, "Fact.World.Flag",
								   "Generic world flags that do not belong to a quest or location.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Location, "Fact.Location",
								   "Root for per location facts: Fact.Location.<LocationId>.<Aspect>.");
}
