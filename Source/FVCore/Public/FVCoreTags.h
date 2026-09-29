#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDCORE_API

namespace FVCoreTags
{
	UE_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// ============================================================================
	// INPUT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Walk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Sprint);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Jump);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Aim);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction_Primary);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction_Secondary);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interaction_Ternary);
	
	// ============================================================================
	// CORE ACTOR TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Pawn);
	
	// ============================================================================
	// FLOW GRAPH
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flow_Common_Enable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flow_Common_Disable);
	
	// ============================================================================
	// COMPONENT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flow_Common_TriggerZone);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flow_Common_InteractionZone);
	
	// ============================================================================
	// FACT ROOT TAGS
	// Fact.<Domain>.<Subject>.<Aspect>. Domain roots live in the module that owns
	// them (narrative, character, world, gameplay, AI); only engine-level roots are here.
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_System);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact_Debug);

}

#undef UE_API