// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVMovementTags.h"
#include "GameplayTagsManager.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

FFVMovementTags FFVMovementTags::Instance;

const FFVMovementTags& FFVMovementTags::Get()
{
	return Instance;
}

FFVMovementTags::FFVMovementTags()
{
	// Tags will be initialized via InitializeNativeTags
}

void FFVMovementTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FFVMovementTags& Tags = Instance;

	// Movement Mode Tags
	Tags.AddTag(Tags.Movement, "Movement", "Base tag for all movement");
	Tags.AddTag(Tags.Movement_Mode_OnGround, "Movement.Mode.OnGround", "Character is on solid ground");
	Tags.AddTag(Tags.Movement_Mode_InAir, "Movement.Mode.InAir", "Character is in the air");
	Tags.AddTag(Tags.Movement_Mode_Traversing, "Movement.Mode.Traversing", "Character is traversing");
	Tags.AddTag(Tags.Movement_Mode_Swimming, "Movement.Mode.Swimming", "Character is swimming");
	Tags.AddTag(Tags.Movement_Mode_Flying, "Movement.Mode.Flying", "Character is flying");
	Tags.AddTag(Tags.Movement_Mode_Custom, "Movement.Mode.Custom", "Custom movement mode");

	// Movement State Tags
	Tags.AddTag(Tags.Movement_State, "Movement.State", "Base tag for movement states");
	Tags.AddTag(Tags.Movement_State_Crouching, "Movement.State.Crouching", "Character is crouching");
	Tags.AddTag(Tags.Movement_State_Sprinting, "Movement.State.Sprinting", "Character is sprinting");
	Tags.AddTag(Tags.Movement_State_Sliding, "Movement.State.Sliding", "Character is sliding");
	Tags.AddTag(Tags.Movement_State_Rooted, "Movement.State.Rooted", "Character is rooted");
	Tags.AddTag(Tags.Movement_State_Slowed, "Movement.State.Slowed", "Movement is slowed");
	Tags.AddTag(Tags.Movement_State_Ragdoll, "Movement.State.Ragdoll", "Character is in ragdoll state");

	// Movement Action Tags
	Tags.AddTag(Tags.Movement_Action, "Movement.Action", "Base tag for movement actions");
	Tags.AddTag(Tags.Movement_Action_Jump, "Movement.Action.Jump", "Character is jumping");
	Tags.AddTag(Tags.Movement_Action_Mantle, "Movement.Action.Mantle", "Character is mantling");
	Tags.AddTag(Tags.Movement_Action_Vault, "Movement.Action.Vault", "Character is vaulting over obstacles");
	Tags.AddTag(Tags.Movement_Action_Hurdle, "Movement.Action.Hurdle", "Character is hurdling over obstacles");
	Tags.AddTag(Tags.Movement_Action_Climb, "Movement.Action.Climb", "Character is climbing");
	Tags.AddTag(Tags.Movement_Action_Roll, "Movement.Action.Roll", "Character is rolling");

	// Movement Restriction Tags
	Tags.AddTag(Tags.Movement_Restriction, "Movement.Restriction", "Base tag for movement restrictions");
	Tags.AddTag(Tags.Movement_Restriction_NoJump, "Movement.Restriction.NoJump", "Cannot jump");
	Tags.AddTag(Tags.Movement_Restriction_NoSprint, "Movement.Restriction.NoSprint", "Cannot sprint");
	Tags.AddTag(Tags.Movement_Restriction_NoCrouch, "Movement.Restriction.NoCrouch", "Cannot crouch");
	Tags.AddTag(Tags.Movement_Restriction_NoDirectionChange, "Movement.Restriction.NoDirectionChange", "Cannot change direction");
	Tags.AddTag(Tags.Movement_Restriction_NoTraversal, "Movement.Restriction.NoTraversal", "Cannot perform traversal");

	// Surface Tags
	Tags.AddTag(Tags.Movement_Surface, "Movement.Surface", "Base tag for surface types");
	Tags.AddTag(Tags.Movement_Surface_Walkable, "Movement.Surface.Walkable", "Standard walkable surface");
	Tags.AddTag(Tags.Movement_Surface_Slippery, "Movement.Surface.Slippery", "Slippery surface");
	Tags.AddTag(Tags.Movement_Surface_Sticky, "Movement.Surface.Sticky", "Sticky surface");
	Tags.AddTag(Tags.Movement_Surface_Climbable, "Movement.Surface.Climbable", "Climbable surface");
	Tags.AddTag(Tags.Movement_Surface_Water, "Movement.Surface.Water", "Water surface");

	UE_LOG(LogFVMovement, Log, TEXT("FlickerVoid Movement Tags Initialized"));
}

void FFVMovementTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}
