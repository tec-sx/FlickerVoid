#include "FVGameplayTags.h"

namespace FVGameplayTags
{
	// ============================================================================
	// INITIALIZATION STATE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	// ============================================================================
	// CHARACTER STATUS TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Interacting, "Status.Interacting", "Actor is currently running an interaction and should suppress movement and further interaction input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_HandsBusy, "Status.HandsBusy", "Both hands are needed by the current action; held items are put away.");


	// ============================================================================
	// ABILITY ACTIVATION FAILURE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	// ============================================================================
	// EFFECT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Effect_Equipment, "Effect.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable, "Effect.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Food, "Effect.Consumable.Food");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Drug, "Effect.Consumable.Drug");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Consumable_Beverage, "Effect.Consumable.Beverage");

	// ============================================================================
	// INTERACTION ACTION TAGS (game actions; the interaction plugin defines the generic ones)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Wear, "Interaction.Action.Wear", "Wear an item.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Hold, "Interaction.Action.Hold", "Hold an item in a hand.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_TakeAll, "Interaction.Action.TakeAll", "Take everything from a container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Search, "Interaction.Action.Search", "Search a container or body.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Pickpocket, "Interaction.Action.Pickpocket", "Steal from a character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Trade, "Interaction.Action.Trade", "Trade with a merchant.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Investigate, "Interaction.Action.Investigate", "Turn an object over to find its secrets.");

	// ============================================================================
	// FACT TAGS (GAMEPLAY)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Gameplay, "Fact.Gameplay", "Root for narrative relevant gameplay facts.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Gameplay_Item, "Fact.Gameplay.Item",
								   "Story relevant item state, e.g. a key item was obtained or handed over.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Gameplay_Interaction, "Fact.Gameplay.Interaction",
								   "Records that a specific interaction happened at least once.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fact_Gameplay_Counter, "Fact.Gameplay.Counter",
								   "Global tallies not owned by a quest, e.g. total civilians saved.");

	// ============================================================================
	// SCANNER TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Scan_Category, "Scan.Category",
								   "Root for scan categories; each one maps to a highlight style in Scanner settings.");

	// ============================================================================
	// ITEM HAND SLOT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Hand_Left, "Equipment.Slot.Hand.Left", "Held in the left hand.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipment_Slot_Hand_Right, "Equipment.Slot.Hand.Right", "Held in the right hand.");

	// ============================================================================
	// ITEM EVENT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Pickup, "Event.Item.Pickup", "Take a world item or money into the inventory.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Use, "Event.Item.Use", "Use an item from the inventory or straight from the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Wear, "Event.Item.Wear", "Wear an item; a world item is picked up first.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Hold, "Event.Item.Hold", "Hold an item in a hand; a world item is picked up first.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Drop, "Event.Item.Drop", "Drop an inventory stack into the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Throw, "Event.Item.Throw", "Throw an item to make noise.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Read, "Event.Item.Read", "Read a note or document.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_TakeAll, "Event.Item.TakeAll", "Take everything from a container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Pickpocket, "Event.Item.Pickpocket", "Steal from a character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Give, "Event.Item.Give", "Hand an item or money to or from another character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_UseHeld, "Event.Item.UseHeld", "Use the item held in a hand, e.g. switch a flashlight on.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Notify_Attach, "Event.Item.Notify.Attach", "Montage moment the item reaches the hand.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Notify_Apply, "Event.Item.Notify.Apply", "Montage moment the item takes effect: used, taken or thrown.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Notify_Release, "Event.Item.Notify.Release", "Montage moment the item leaves the hand.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Item_Notify_Handover, "Event.Item.Notify.Handover", "Montage moment the item passes to the other character's hand.");

	// ============================================================================
	// ITEM MESSAGE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Item_Read, "Message.Item.Read", "A note or document was read (FFVItemReadMessage).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Item_GiveFinished, "Message.Item.GiveFinished", "A give ended, given or not (FFVItemGiveMessage).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Item_Stolen, "Message.Item.Stolen", "Something was stolen, or a theft was noticed (FFVItemStolenMessage).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Item_Noise, "Message.Item.Noise", "A thrown item made noise (FFVItemNoiseMessage).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Item_ContainerOpened, "Message.Item.ContainerOpened", "A container was opened (FFVItemContainerMessage).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Item_TradeRequested, "Message.Item.TradeRequested", "A merchant was asked to trade (FFVItemContainerMessage).");

	// ============================================================================
	// ITEM ANIMATION STYLE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Use, "Animation.Item.Use", "Root of item use styles; children are data, e.g. Animation.Item.Use.Drink.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Use_Read, "Animation.Item.Use.Read", "Reading a note or document.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Hold, "Animation.Item.Hold", "Root of item hold styles: upper body poses for held items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Pickup_Low, "Animation.Item.Pickup.Low", "Picking up an item near the floor.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Pickup_Mid, "Animation.Item.Pickup.Mid", "Picking up an item at waist height.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Pickup_High, "Animation.Item.Pickup.High", "Picking up an item above the shoulders.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Give, "Animation.Item.Give", "Handing an item to another character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Animation_Item_Throw, "Animation.Item.Throw", "Throwing an item.");
}
