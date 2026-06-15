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
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_ExamineStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_LockpickStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_LockpickEnded);

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
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Character);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Device);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Document);
}

#undef UE_API
