#include "Character/FVCharacterTags.h"

namespace FVCharacterTags
{
	// ============================================================================
	// MOVEMENT MODE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_OnGround, "Movement.Mode.OnGround", "Character on ground.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_InAir, "Movement.Mode.InAir", "Character in air.");

	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking,		Movement_Mode_OnGround },
		{ MOVE_NavWalking,	Movement_Mode_OnGround },
		{ MOVE_Flying,		Movement_Mode_OnGround },
		{ MOVE_Custom,		Movement_Mode_OnGround },
		{ MOVE_Falling,		Movement_Mode_InAir },
		{ MOVE_Swimming,	Movement_Mode_InAir },
	};

	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap = {};

	// ============================================================================
	// MOVEMENT ACTION TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Action_Jump, "Movement.Action.Jump", "Action to jump.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Action_Traverse, "Movement.Action.Traverse", "Action to check for traversal opportunities.");

	// ============================================================================
	// MOVEMENT GAIT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Gait_Walking, "Movement.Gait.Walking", "Walking gait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Gait_Jogging, "Movement.Gait.Jogging", "Jogging gait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Gait_Sprinting, "Movement.Gait.Sprinting", "Sprinting gait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Gait_Crawling, "Movement.Gait.Crawling", "Crawling gait.");

	// ============================================================================
	// MOVEMENT STANCE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Stance_Standing, "Movement.Stance.Standing", "Standing stance.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Stance_Crouching, "Movement.Stance.Crouching", "Crouching stance.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Stance_Prone, "Movement.Stance.Prone", "Prone stance.");

	// ============================================================================
	// MOVEMENT STATUS TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_Idle, "Movement.Status.Idle", "Character idle.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_Moving, "Movement.Status.Moving", "Character moving.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_JustLanded, "Movement.Status.JustLanded", "Just landed from air.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_Starting, "Movement.Status.Starting", "Starting to move.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_Stopping, "Movement.Status.Stopping", "Stopping movement.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_Pivoting, "Movement.Status.Pivoting", "Pivoting direction.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_TurningInPlace, "Movement.Status.TurningInPlace", "Turning in place.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_HasAcceleration, "Movement.Status.HasAcceleration", "Has input acceleration.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Status_HasVelocity, "Movement.Status.HasVelocity", "Has velocity.");

	// ============================================================================
	// ROTATION MODE TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Rotation_Mode_OrientToMovement, "Rotation.Mode.OrientToMovement", "Orient to movement.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Rotation_Mode_Strafe, "Rotation.Mode.Strafe", "Strafe mode.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Rotation_Mode_LookAt, "Rotation.Mode.LookAt", "Look at target.");
	
	// ============================================================================
	// CHARACTER TAGS (Player/AI input Actions)
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Action_Aim, "Character.Action.Aim", "Action to aim.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Action_Interact, "Character.Action.Interact", "Action to interact.");
	
	// ============================================================================
	// CHARACTER TRAIT TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Aggressive, "Character.Trait.Aggressive", "Aggressive trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Cowardly, "Character.Trait.Cowardly", "Cowardly trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Greedy, "Character.Trait.Greedy", "Greedy trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Honorable, "Character.Trait.Honorable", "Honorable trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Corrupt, "Character.Trait.Corrupt", " Corrupt trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Brave, "Character.Trait.Brave", "Brave trait.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Trait_Cautious, "Character.Trait.Cautious", "Cautious trait.");

	// ============================================================================
	// CHARACTER MOOD TAGS
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Happy, "Character.Mood.Happy", "Happy mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Angry, "Character.Mood.Angry", "Angry mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Fearful, "Character.Mood.Fearful", "Fearful mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Intoxicated, "Character.Mood.Intoxicated", "Intoxicated mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Stressed, "Character.Mood.Stressed", "Stressed mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Horny, "Character.Mood.Horny", "Aroused mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Exhausted, "Character.Mood.Exhausted", "Exhausted mood.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Mood_Energized, "Character.Mood.Energized", "Energized mood.");
}
