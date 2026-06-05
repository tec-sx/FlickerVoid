// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * FVMovementTags
 * Centralized gameplay tags for the movement system
 * Following GAS-like tag architecture for movement states and abilities
 */
struct FLICKERVOIDCORE_API FFVMovementTags
{
public:
	// Initialize all tags - called during module startup
	static void InitializeNativeTags();

	// Get the singleton instance
	static const FFVMovementTags& Get();

	//~=============================================================================
	// Movement Mode Tags
	//~=============================================================================
	
	/** Base tag for all movement modes */
	FGameplayTag Movement;
	
	/** Character is standing on solid ground */
	FGameplayTag Movement_Mode_OnGround;
	
	/** Character is in the air (jumping, falling) */
	FGameplayTag Movement_Mode_InAir;
	
	/** Character is traversing (climbing, vaulting, etc.) */
	FGameplayTag Movement_Mode_Traversing;
	
	/** Character is swimming */
	FGameplayTag Movement_Mode_Swimming;
	
	/** Character is flying */
	FGameplayTag Movement_Mode_Flying;
	
	/** Custom movement mode */
	FGameplayTag Movement_Mode_Custom;

	//~=============================================================================
	// Movement State Tags
	//~=============================================================================
	
	/** Base tag for movement states */
	FGameplayTag Movement_State;
	
	/** Character is crouching */
	FGameplayTag Movement_State_Crouching;
	
	/** Character is sprinting */
	FGameplayTag Movement_State_Sprinting;
	
	/** Character is sliding */
	FGameplayTag Movement_State_Sliding;

	/** Character is rooted in place */
	FGameplayTag Movement_State_Rooted;

	/** Character movement is slowed */
	FGameplayTag Movement_State_Slowed;

	/** Character is in ragdoll state */
	FGameplayTag Movement_State_Ragdoll;

	//~=============================================================================
	// Movement Action Tags
	//~=============================================================================
	
	/** Base tag for movement actions */
	FGameplayTag Movement_Action;
	
	/** Character is jumping */
	FGameplayTag Movement_Action_Jump;
	
	/** Character is mantling */
	FGameplayTag Movement_Action_Mantle;

	/** Character is vaulting over obstacles */
	FGameplayTag Movement_Action_Vault;

	/** Character is hurdling over obstacles */
	FGameplayTag Movement_Action_Hurdle;

	/** Character is climbing */
	FGameplayTag Movement_Action_Climb;

	/** Character is rolling */
	FGameplayTag Movement_Action_Roll;

	//~=============================================================================
	// Movement Restriction Tags
	//~=============================================================================
	
	/** Base tag for movement restrictions */
	FGameplayTag Movement_Restriction;
	
	/** Cannot jump */
	FGameplayTag Movement_Restriction_NoJump;
	
	/** Cannot sprint */
	FGameplayTag Movement_Restriction_NoSprint;
	
	/** Cannot crouch */
	FGameplayTag Movement_Restriction_NoCrouch;
	
	/** Cannot change direction */
	FGameplayTag Movement_Restriction_NoDirectionChange;
	
	/** Cannot perform traversal actions */
	FGameplayTag Movement_Restriction_NoTraversal;

	//~=============================================================================
	// Surface Tags (for ground detection)
	//~=============================================================================
	
	/** Base tag for surface types */
	FGameplayTag Movement_Surface;

	/** Default walkable surface */
	FGameplayTag Movement_Surface_Walkable;

	/** Slippery surface (ice, oil) */
	FGameplayTag Movement_Surface_Slippery;

	/** Sticky surface (tar, mud) */
	FGameplayTag Movement_Surface_Sticky;

	/** Climbable surface */
	FGameplayTag Movement_Surface_Climbable;

	/** Water surface */
	FGameplayTag Movement_Surface_Water;

protected:
	// Add tags to the global dictionary
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);

private:
	FFVMovementTags(); // Private constructor
	static FFVMovementTags Instance;
};
