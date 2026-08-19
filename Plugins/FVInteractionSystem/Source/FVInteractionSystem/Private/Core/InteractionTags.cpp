#include "Core/InteractionTags.h"

namespace InteractionTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_OfferChanged, "Interaction.OfferChanged", "Broadcast when the player's active interaction offer changes.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_ExamineStarted, "Interaction.Event.ExamineStarted", "Fired when an examine overlay should open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_LockpickStarted, "Interaction.Event.LockpickStarted", "Fired when a lockpick mini-game should open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_LockpickEnded, "Interaction.Event.LockpickEnded", "Fired when the lockpick mini-game closes.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Pickup, "Interaction.Action.Pickup", "Pick up an item from the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Use, "Interaction.Action.Use", "Use/activate an interactable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Open, "Interaction.Action.Open", "Open a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Close, "Interaction.Action.Close", "Close a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Talk, "Interaction.Action.Talk", "Talk to a character.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Examine, "Interaction.Action.Examine", "Examine/inspect an object.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Read, "Interaction.Action.Read", "Read a document/note.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Activate, "Interaction.Action.Activate", "Activate a device/switch.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_WalkedAway, "Interaction.Cancel.WalkedAway", "Interaction aborted because the player left the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_HigherPriorityOffer, "Interaction.Cancel.HigherPriorityOffer", "Interaction aborted for a higher priority offer.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_OfferExpired, "Interaction.Cancel.OfferExpired", "Interaction aborted because the offer expired.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_CombatStarted, "Interaction.Cancel.CombatStarted", "Interaction aborted because combat started.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Death, "Interaction.Cancel.Death", "Interaction aborted because the instigator died.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Scripted, "Interaction.Cancel.Scripted", "Interaction aborted by script.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item, "Interactable.Item", "An item that can be interacted with.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item_Pickup, "Interactable.Item.Pickup", "An item pickup in the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Door, "Interactable.Door", "A door that can be opened/closed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Container, "Interactable.Container", "A container with items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Character, "Interactable.Character", "A Character that can be talked to.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Device, "Interactable.Device", "A device that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Document, "Interactable.Document", "A readable document.");
}