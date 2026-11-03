#include "Core/FVInteractionGameplayTags.h"

namespace FVInteractionGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action, "Interaction.Action", "Root of interaction action tags.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction, "InputTag.Interaction", "Root of interaction input tags.");
	
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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_LockPick, "Interaction.Action.LockPick", "Attempt to pick a lock.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_WalkedAway, "Interaction.Cancel.WalkedAway", "Interaction aborted because the player left the target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_OfferExpired, "Interaction.Cancel.OfferExpired", "Interaction aborted because the offer expired.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Scripted, "Interaction.Cancel.Scripted", "Interaction aborted by script.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_FocusLost, "Interaction.Cancel.FocusLost", "Interaction aborted because the interactor looked away.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Suppressed, "Interaction.Cancel.Suppressed", "Interaction aborted because the interactor became suppressed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_RequirementFailed, "Interaction.Cancel.RequirementFailed", "Interaction aborted because a requirement stopped being met.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Player, "Interaction.Cancel.Player", "Interaction aborted by an explicit player cancel input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Released, "Interaction.Cancel.Released", "Hold interaction aborted because the key was released early.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Timeout, "Interaction.Cancel.Timeout", "Mash interaction aborted because the window elapsed.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Primary, "InputTag.Interaction.Primary", "Primary interaction input slot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Secondary, "InputTag.Interaction.Secondary", "Secondary interaction input slot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Ternary, "InputTag.Interaction.Ternary", "Third interaction input slot.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Suppression_Cutscene, "Interaction.Suppression.Cutscene", "Suppressed for the duration of a cutscene.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Suppression_Dependency, "Interaction.Suppression.Dependency", "Suppressed by another interaction holding a dependency.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactor_Tag_Player, "Interactor.Tag.Player", "Interactor driven by a human player.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactor_Mode, "Interactor.Mode", "Root for interactor detection modes.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactor_Mode_Default, "Interactor.Mode.Default", "Default interactor detection mode.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactor_Mode_Aim, "Interactor.Mode.Aim", "Precise detection mode used while aiming.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable, "Interactable", "Base interactable tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item, "Interactable.Item", "An item that can be interacted with.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item_Pickup, "Interactable.Item.Pickup", "An item pickup in the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Door, "Interactable.Door", "A door that can be opened/closed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Container, "Interactable.Container", "A container with items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Character, "Interactable.Character", "A Character that can be talked to.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Device, "Interactable.Device", "A device that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Document, "Interactable.Document", "A readable document.");
}