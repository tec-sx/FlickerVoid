#pragma once

#include "NativeGameplayTags.h"

#define UE_API FVINTERACTIONSYSTEM_API

namespace InteractionTags
{
	// Runtime events fired by handlers — UI/Blueprint listens for these
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_OfferChanged);
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

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_WalkedAway);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_HigherPriorityOffer);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_OfferExpired);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_CombatStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_Death);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Cancel_Scripted);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item_Pickup);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Door);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Container);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Character);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Device);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Document);
}

#undef UE_API