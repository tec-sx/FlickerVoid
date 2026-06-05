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
}
