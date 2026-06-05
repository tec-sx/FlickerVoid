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

	// ============================================================================
	// INTERACTION ACTION TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Pickup);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Use);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Open);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Close);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Talk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Examine);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Read);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Activate);

	// ============================================================================
	// INTERACTABLE TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item_Pickup);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Door);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Container);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_NPC);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Device);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Document);
}

#undef UE_API