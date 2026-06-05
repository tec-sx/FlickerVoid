#pragma once
#include "GameplayTagContainer.h"
#include "Character/FVCharacterTags.h"
#include "FVAnimationTypes.generated.h"

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVCharacterAnimationData
{
    GENERATED_BODY();

	//~=============================================================================
	// Character State (Tag Reference - NO COPY!)
	//~=============================================================================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FGameplayTagContainer CharacterTags; 
	
	//~=============================================================================
	// Transform & Physics Data (Always needed by animation)
	//~=============================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transform")
	FTransform ActorTransform;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	FVector InputAcceleration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	float GroundSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	float CurrentMaxAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	float CurrentMaxDeceleration;
	
	//~=============================================================================
	// Rotation Intent
	//~=============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rotation")
	FRotator OrientationIntent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rotation")
	FRotator AimingDirection;

	//~=============================================================================
	// Landing Data
	//~=============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing")
	FVector LandingVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground")
	FVector GroundNormal;
	
	//~=============================================================================
	// Convenience Queries (Blueprint helpers - dereference pointer and query tags)
	//~=============================================================================

	/** Is character sprinting? (checks for Movement.Gait.Sprinting tag) */
	FORCEINLINE bool IsSprinting() const
	{
		return CharacterTags.HasTag(FVCharacterTags::Movement_Gait_Sprinting);
	}

	/** Is character in air? (checks for Movement.Mode.InAir tag) */
	FORCEINLINE bool IsInAir() const
	{
		return CharacterTags.HasTag(FVCharacterTags::Movement_Mode_InAir);
	}

	/** Is character crouching? (checks for Movement.Stance.Crouching tag) */
	FORCEINLINE bool IsCrouching() const
	{
		return CharacterTags.HasTag(FVCharacterTags::Movement_Stance_Crouching);
	}

	/** Just landed? (checks for Movement.Status.JustLanded tag) */
	FORCEINLINE bool JustLanded() const
	{
		return CharacterTags.HasTag(FVCharacterTags::Movement_Status_JustLanded);
	}

	/** Is idle? (checks for Movement.Status.Idle tag) */
	FORCEINLINE bool IsIdle() const
	{
		return CharacterTags.HasTag(FVCharacterTags::Movement_Status_Idle);
	}

	/** Check if has specific tag (generic helper) */
	FORCEINLINE bool HasTag(const FGameplayTag& Tag) const
	{
		return CharacterTags.HasTag(Tag);
	}

	/** Check if has all tags */
	FORCEINLINE bool HasAllTags(const FGameplayTagContainer& Tags) const
	{
		return CharacterTags.HasAll(Tags);
	}

	/** Check if has any tags */
	FORCEINLINE bool HasAnyTags(const FGameplayTagContainer& Tags) const
	{
		return CharacterTags.HasAny(Tags);
	}
};