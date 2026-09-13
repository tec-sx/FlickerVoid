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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_HigherPriorityOffer, "Interaction.Cancel.HigherPriorityOffer", "Interaction aborted for a higher priority offer.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_OfferExpired, "Interaction.Cancel.OfferExpired", "Interaction aborted because the offer expired.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_CombatStarted, "Interaction.Cancel.CombatStarted", "Interaction aborted because combat started.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Death, "Interaction.Cancel.Death", "Interaction aborted because the instigator died.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Scripted, "Interaction.Cancel.Scripted", "Interaction aborted by script.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_FocusLost, "Interaction.Cancel.FocusLost", "Interaction aborted because the interactor looked away.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Suppressed, "Interaction.Cancel.Suppressed", "Interaction aborted because the interactor became suppressed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_RequirementFailed, "Interaction.Cancel.RequirementFailed", "Interaction aborted because a requirement stopped being met.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Player, "Interaction.Cancel.Player", "Interaction aborted by an explicit player cancel input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Released, "Interaction.Cancel.Released", "Hold interaction aborted because the key was released early.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Cancel_Timeout, "Interaction.Cancel.Timeout", "Mash interaction aborted because the window elapsed.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Primary, "InputTag.Interaction.Primary", "Primary interaction input slot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Secondary, "InputTag.Interaction.Secondary", "Secondary interaction input slot.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interaction_Alternate, "InputTag.Interaction.Alternate", "Alternate interaction input slot.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Idle, "Interaction.State.Idle", "Interactable is out of range of every interactor.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Awake, "Interaction.State.Awake", "Interactable is in range and can react.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Suppressed, "Interaction.State.Suppressed", "Interactable is deliberately blocked.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Interacting, "Interaction.State.Interacting", "Interactable is currently in use.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Paused, "Interaction.State.Paused", "Interaction is paused, waiting for input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Cooldown, "Interaction.State.Cooldown", "Interactable is cooling down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_State_Completed, "Interaction.State.Completed", "Interactable is spent and cannot be used again.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Suppression_Cutscene, "Interaction.Suppression.Cutscene", "Suppressed for the duration of a cutscene.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Suppression_Menu, "Interaction.Suppression.Menu", "Suppressed while a menu is open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Suppression_Vehicle, "Interaction.Suppression.Vehicle", "Suppressed while driving or riding.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Suppression_Dependency, "Interaction.Suppression.Dependency", "Suppressed by another interaction holding a dependency.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactor_Tag_Player, "Interactor.Tag.Player", "Interactor driven by a human player.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactor_Tag_AI, "Interactor.Tag.AI", "Interactor driven by AI.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item, "Interactable.Item", "An item that can be interacted with.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Item_Pickup, "Interactable.Item.Pickup", "An item pickup in the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Door, "Interactable.Door", "A door that can be opened/closed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Container, "Interactable.Container", "A container with items.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Character, "Interactable.Character", "A Character that can be talked to.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Device, "Interactable.Device", "A device that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Document, "Interactable.Document", "A readable document.");

	FGameplayTag StateToTag(EFVInteractableState State)
	{
		switch (State)
		{
		case EFVInteractableState::Idle:			return Interaction_State_Idle;
		case EFVInteractableState::Awake:			return Interaction_State_Awake;
		case EFVInteractableState::Suppressed:	return Interaction_State_Suppressed;
		case EFVInteractableState::Interacting:	return Interaction_State_Interacting;
		case EFVInteractableState::Paused:		return Interaction_State_Paused;
		case EFVInteractableState::Cooldown:		return Interaction_State_Cooldown;
		case EFVInteractableState::Completed:		return Interaction_State_Completed;
		default:								return FGameplayTag();
		}
	}
}