#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDCORE_API

namespace FVCoreTags
{
	UE_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

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

	// Four interaction input slots — physical key binding lives only in the InputConfig data asset
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact_Primary);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact_PrimaryHold);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact_Secondary);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact_SecondaryHold);

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
	// INTERACTION ACTION TAGS
	// ============================================================================
	// Runtime events fired by handlers — UI/Blueprint listens for these
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_ExamineStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_LockpickStarted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Event_LockpickEnded);

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Pickup);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Use);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Open);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Close);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Talk);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Examine);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Read);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interaction_Action_Activate);

	// ============================================================================
	// INTERACTABLE TYPE TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Item_Pickup);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Door);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Container);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_NPC);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Device);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interactable_Document);

	// ============================================================================
	// ITEM REQUIREMENT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Protagonist);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Level_1);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Level_10);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Level_20);

	// ============================================================================
	// CHARACTER STATE TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Idle);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Alert);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Suspicious);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Hostile);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Combat);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Fleeing);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Incapacitated);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead);

	// ============================================================================
	// NPC BEHAVIOR DECISION TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Ignore);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Greet);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Observe);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_VerbalConfrontation);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Threaten);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Demand);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Attack);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_CallForHelp);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Behavior_Decision_Flee);

	// ============================================================================
	// ATTACK STYLE TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_BerserkerCharge);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_CalculatedStrike);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_RangedAmbush);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_Grapple);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_CautiousProbe);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Style_CowardlyBackstab);

	// ============================================================================
	// PLAYER RESPONSE TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Defend);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Attack);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Intimidate);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Persuade);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Bribe);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Seduce);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Submit);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Threaten);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Response_Mock);

	// ============================================================================
	// CHARACTER MOOD TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Happy);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Angry);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Fearful);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Intoxicated);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Stressed);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Horny);

	// ============================================================================
	// CHARACTER TRAIT TAGS (for gameplay effect conditions)
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Aggressive);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Cowardly);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Greedy);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Honorable);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Corrupt);

	// ============================================================================
	// NARRATIVE & QUEST TAGS
	// ============================================================================
	// Quest States
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Unavailable);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Available);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Active);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Completed);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Failed);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_State_Abandoned);

	// Quest Types
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_MainStory);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Side);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Memory);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Rehabilitation);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Errand);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest_Type_Social);

	// Objective Types
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Talk);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Investigate);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Infiltrate);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Obtain);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Deliver);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Attend);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Wait);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Threshold);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_MaintainState);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Objective_Type_Location);

	// ============================================================================
	// SANITY & PSYCHOLOGICAL STATE TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Stable);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Anxious);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Disturbed);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Unstable);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Breaking);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sanity_State_Broken);

	// Addiction States
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Clean);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Casual);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Dependent);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Addicted);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Withdrawal);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Addiction_State_Recovering);

	// Substance Types
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Alcohol);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Sedatives);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Stimulants);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Substance_Hallucinogens);

	// ============================================================================
	// MEMORY & IDENTITY TAGS
	// ============================================================================

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Identity);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Relationship);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Skill);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Trauma);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Type_Location);

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Clarity_Fragment);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Clarity_Partial);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Memory_Clarity_Clear);

	// ============================================================================
	// WORLD STATE & CONSEQUENCE TAGS
	// ============================================================================

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldState_Chapter_1);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldState_Chapter_2);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(WorldState_Chapter_3);

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Reputation_Gain);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Reputation_Loss);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Trust_Gain);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Trust_Loss);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Memory_Unlock);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consequence_Sanity_Impact);

	// ============================================================================
	// NARRATIVE PROGRESSION BLOCKERS
	// ============================================================================

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Sanity_TooLow);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Addiction_Active);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Trust_Insufficient);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Memory_Required);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocker_Rehabilitation_Required);

	// ============================================================================
	// EFFECT TAGS
	// ============================================================================
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Equipment);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Food);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Drug);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Consumable_Beverage);
}

#undef UE_API