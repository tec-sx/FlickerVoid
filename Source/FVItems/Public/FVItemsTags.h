#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDITEMS_API

namespace FVItemsTags
{
	// ============================================================================
	// ITEM TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Drink);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Food);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_Drug);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Quest);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Lore);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Cosmetic);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Outfit);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Junk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Misc);
	
	// ============================================================================
	// ITEM EFFECT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemEffect_Clarity_Down);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemEffect_Clarity_Up);
}

#undef UE_API