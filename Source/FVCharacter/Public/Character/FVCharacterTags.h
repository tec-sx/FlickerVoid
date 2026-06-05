#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDCHARACTER_API

namespace FVCharacterTags
{
	// ============================================================================
	// MOVEMENT MODE TAGS (Auto-synced from CMC)
	// ============================================================================
	UE_API	extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	UE_API	extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_OnGround);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_InAir);

	// ============================================================================
	// MOVEMENT ACTION TAGS (Player/AI input Actions)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Action_Jump);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Action_Traverse);
	
	// ============================================================================
	// MOVEMENT GAIT TAGS (Active gait state)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Gait_Walking);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Gait_Jogging);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Gait_Sprinting);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Gait_Crawling);
	
	// ============================================================================
	// MOVEMENT STANCE TAGS (Auto-synced, replaces EFVStance enum)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Stance_Standing);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Stance_Crouching);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Stance_Prone);
	
	// ============================================================================
	// MOVEMENT STATUS TAGS (Auto-synced, replaces booleans)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_Idle);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_Moving);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_JustLanded);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_Starting);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_Stopping);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_Pivoting);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_TurningInPlace);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_HasAcceleration);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Status_HasVelocity);
	
	// ============================================================================
	// ROTATION MODE TAGS (Intent-based, replaces EFVRotationMode enum)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rotation_Mode_OrientToMovement);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rotation_Mode_Strafe);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rotation_Mode_LookAt);
	
	// ============================================================================
	// CHARACTER TAGS (Player/AI input Actions)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_Aim);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_Interact);

	// ============================================================================
	// CHARACTER TRAIT TAGS (Permanent characteristics)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Aggressive);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Cowardly);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Greedy);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Honorable);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Corrupt);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Brave);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Trait_Cautious);

	// ============================================================================
	// CHARACTER MOOD TAGS (Temporary emotional states)
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Happy);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Angry);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Fearful);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Intoxicated);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Stressed);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Horny);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Exhausted);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mood_Energized);
}

#undef UE_API
