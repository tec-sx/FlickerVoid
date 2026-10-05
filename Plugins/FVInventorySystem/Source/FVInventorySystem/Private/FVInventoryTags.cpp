#include "FVInventoryTags.h"

namespace FVInventoryTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item, "Item", "Item categories.");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable, "Item.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Quest, "Item.Quest");
	UE_DEFINE_GAMEPLAY_TAG(Item_Lore, "Item.Lore");
	UE_DEFINE_GAMEPLAY_TAG(Item_Outfit, "Item.Outfit");
	UE_DEFINE_GAMEPLAY_TAG(Item_Misc, "Item.Misc");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot, "Equipment.Slot", "Equipment slots.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Outfit, "Equipment.Slot.Outfit", "Full-body outfit.");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Head, "Equipment.Slot.Head");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Eyes, "Equipment.Slot.Eyes");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Bag, "Equipment.Slot.Bag");
}
