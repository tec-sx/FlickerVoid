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
	// INTERACTION ACTION TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_ExamineStarted,  "Interaction.Event.ExamineStarted",  "Fired when an examine overlay should open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_LockpickStarted, "Interaction.Event.LockpickStarted", "Fired when a lockpick mini-game should open.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Event_LockpickEnded,   "Interaction.Event.LockpickEnded",   "Fired when the lockpick mini-game closes.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Pickup, "Interaction.Action.Pickup", "Pick up an item from the world.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Use, "Interaction.Action.Use", "Use/activate an interactable.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Open, "Interaction.Action.Open", "Open a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Close, "Interaction.Action.Close", "Close a door/container.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interaction_Action_Talk, "Interaction.Action.Talk", "Talk to a character.");
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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Character, "Interactable.Character", "A Character that can be talked to.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Device, "Interactable.Device", "A device that can be activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Interactable_Document, "Interactable.Document", "A readable document.");
}
