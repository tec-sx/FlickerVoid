#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

namespace FVGameplayTags
{
	// ============================================================================
	// INITIALIZATION STATE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

	// ============================================================================
	// CHARACTER STATUS TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Interacting);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_HandsBusy);

	// ============================================================================
	// ABILITY ACTIVATION FAILURE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

	// ============================================================================
	// EFFECT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Equipment);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Food);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Drug);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Beverage);

	// ============================================================================
	// INTERACTION ACTION TAGS
	// ============================================================================

	// Runtime events fired by handlers — UI/Blueprint listens for these
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_OfferChanged);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_ExamineStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_LockpickStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_LockpickEnded);
	
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Scan_Category);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Relevance_ScanTarget);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Pickup);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Use);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Open);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Close);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Talk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Examine);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Read);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Activate);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Wear);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Hold);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_TakeAll);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Search);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Pickpocket);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Trade);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Investigate);

	// Applied to the instigator ASC while an engaged interaction is being aborted,
	// so the dispatched ability can branch on the reason inside EndAbility.
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_WalkedAway);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_HigherPriorityOffer);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_OfferExpired);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_CombatStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_Death);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_Scripted);

	// ============================================================================
	// INTERACTABLE TYPE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item_Pickup);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Door);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Container);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Character);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Device);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Document);

	// ============================================================================
	// FACT TAGS (GAMEPLAY)
	// For narrative relevant gameplay state only. Live inventory and cooldowns stay
	// in their own systems; facts record what the story needs to branch on.
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Gameplay);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Gameplay_Item);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Gameplay_Interaction);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Gameplay_Counter);

	// ============================================================================
	// ITEM HAND SLOT TAGS
	// Equipment slots of held items; the inventory plugin's tag ini lists them too.
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Hand_Left);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment_Slot_Hand_Right);

	// ============================================================================
	// ITEM EVENT TAGS
	// Sent to the acting character with SendGameplayEventToActor; item abilities
	// trigger on them. Notify events come from montages (FV Gameplay Event).
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Pickup);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Use);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Wear);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Hold);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Drop);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Throw);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Read);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_TakeAll);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Pickpocket);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Give);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_UseHeld);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Notify_Attach);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Notify_Apply);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Notify_Release);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Item_Notify_Handover);

	// ============================================================================
	// ITEM MESSAGE TAGS
	// GameplayMessageSubsystem channels for UI and other systems.
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Item_Read);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Item_GiveFinished);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Item_Stolen);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Item_Noise);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Item_ContainerOpened);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Item_TradeRequested);

	// ============================================================================
	// ITEM ANIMATION STYLE TAGS
	// Items name a style; each character maps styles to montages and poses.
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Use);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Use_Read);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Hold);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Pickup_Low);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Pickup_Mid);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Pickup_High);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Give);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Item_Throw);
}

#undef UE_API
