#include "FVItemsTags.h"

namespace FVItemsTags
{
	// ============================================================================
	// ITEM TYPE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable, "Item.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_Drink, "Item.Consumable.Drink");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_Food, "Item.Consumable.Food");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_Drug, "Item.Consumable.Drug");
	UE_DEFINE_GAMEPLAY_TAG(Item_Quest, "Item.Quest");
	UE_DEFINE_GAMEPLAY_TAG(Item_Lore, "Item.Lore");
	UE_DEFINE_GAMEPLAY_TAG(Item_Cosmetic, "Item.Cosmetic");
	UE_DEFINE_GAMEPLAY_TAG(Item_Outfit, "Item.Outfit");
	UE_DEFINE_GAMEPLAY_TAG(Item_Junk, "Item.Junk");
	UE_DEFINE_GAMEPLAY_TAG(Item_Misc, "Item.Misc");

	// ============================================================================
	// ITEM EFFECT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(ItemEffect_Clarity_Down, "ItemEffect.Clarity.Down");
	UE_DEFINE_GAMEPLAY_TAG(ItemEffect_Clarity_Up, "ItemEffect.Clarity.Up");

	// ============================================================================
	// INTERACTION ACTION TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Pickup, "Interaction.Action.Pickup", "Pick up an item from the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Use, "Interaction.Action.Use", "Use/activate an interactable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Open, "Interaction.Action.Open", "Open a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Close, "Interaction.Action.Close", "Close a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Talk, "Interaction.Action.Talk", "Talk to an NPC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Examine, "Interaction.Action.Examine", "Examine/inspect an object.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Read, "Interaction.Action.Read", "Read a document/note.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Activate, "Interaction.Action.Activate", "Activate a device/switch.");

	// ============================================================================
	// INTERACTABLE TYPE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item, "Interactable.Item", "An item that can be interacted with.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item_Pickup, "Interactable.Item.Pickup", "An item pickup in the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Door, "Interactable.Door", "A door that can be opened/closed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Container, "Interactable.Container", "A container with items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_NPC, "Interactable.NPC", "An NPC that can be talked to.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Device, "Interactable.Device", "A device that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Document, "Interactable.Document", "A readable document.");
}