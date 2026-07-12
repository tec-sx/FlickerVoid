#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDAI_API

namespace FVAITags
{
	// ============================================================================
	// CHARACTER STATE TAGS (AI Behavior States)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Idle);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Alert);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Suspicious);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Hostile);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Combat);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Fleeing);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Incapacitated);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead);

	// ============================================================================
	// NPC BEHAVIOR DECISION TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Ignore);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Greet);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Observe);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_VerbalConfrontation);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Threaten);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Demand);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Attack);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_CallForHelp);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Flee);

	// ============================================================================
	// ATTACK STYLE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_BerserkerCharge);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_CalculatedStrike);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_RangedAmbush);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_Grapple);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_CautiousProbe);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_CowardlyBackstab);

	// ============================================================================
	// PLAYER RESPONSE TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Defend);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Attack);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Intimidate);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Persuade);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Bribe);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Seduce);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Submit);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Threaten);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Mock);
	
	
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AI_Event_Idle);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AI_Event_Patrol);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AI_Event_Notice);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AI_Event_Focus);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AI_Event_Act);
}

#undef UE_API
