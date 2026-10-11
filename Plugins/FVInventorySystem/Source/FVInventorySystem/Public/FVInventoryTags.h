#pragma once

#include "NativeGameplayTags.h"

/** Roots the plugin filters on. Concrete categories and slots are data: see Config/Tags/FVInventoryTags.ini. */
namespace FVInventoryTags
{
	FVINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item);
	FVINVENTORYSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot);
}
