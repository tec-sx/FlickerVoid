#include "FVInventoryTags.h"

namespace FVInventoryTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item, "Item", "Item categories.");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable, "Item.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Quest, "Item.Quest");
	UE_DEFINE_GAMEPLAY_TAG(Item_Lore, "Item.Lore");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Outfit, "Item.Outfit", "Worn clothing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Container, "Item.Container", "Bags and backpacks that carry other items.");
	UE_DEFINE_GAMEPLAY_TAG(Item_Misc, "Item.Misc");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot, "Equipment.Slot", "Equipment slots.");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Head, "Equipment.Slot.Head");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Eyewear, "Equipment.Slot.Eyewear");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Underwear_Top, "Equipment.Slot.Underwear.Top");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Underwear_Bottom, "Equipment.Slot.Underwear.Bottom");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Socks, "Equipment.Slot.Socks");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_UpperBody, "Equipment.Slot.UpperBody");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_LowerBody, "Equipment.Slot.LowerBody");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Coat, "Equipment.Slot.Coat", "Jacket or coat worn over the upper body.");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Gloves, "Equipment.Slot.Gloves");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Shoes, "Equipment.Slot.Shoes");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Head, "Equipment.Slot.Accessory.Head");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Neck, "Equipment.Slot.Accessory.Neck");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Arm_Left, "Equipment.Slot.Accessory.Arm.Left");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Arm_Right, "Equipment.Slot.Accessory.Arm.Right");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Leg_Left, "Equipment.Slot.Accessory.Leg.Left");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Leg_Right, "Equipment.Slot.Accessory.Leg.Right");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Accessory_Body, "Equipment.Slot.Accessory.Body");

	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Bag, "Equipment.Slot.Bag");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot_Backpack, "Equipment.Slot.Backpack");
}
