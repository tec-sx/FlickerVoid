// Copyright FlickerVoid. All Rights Reserved.

#include "Character/FVCharacterStateManager.h"
#include "Character/FVCharacter.h"
#include "Character/FVCharacterTags.h"
#include "Movement/FVMovementTags.h"
#include "GameFramework/CharacterMovementComponent.h"

UFVCharacterStateManager::UFVCharacterStateManager()
	: OwningCharacter(nullptr)
	, bAutoSyncMovementTags(true)
	, bAutoSyncRotationTags(true)
	, bAutoSyncStatusTags(true)
{
}

void UFVCharacterStateManager::Initialize(AFVCharacter* InOwner)
{
	OwningCharacter = InOwner;
	
	// Initialize with default tags
	Tags.Reset();
	PreviousTags.Reset();
	TagCounts.Reset();

	// Set initial state
	AddTagInternal(FVCharacterTags::Movement_Mode_OnGround, false);
	AddTagInternal(FVCharacterTags::Movement_Gait_Walking, false);
	AddTagInternal(FVCharacterTags::Movement_Status_Idle, false);
}

void UFVCharacterStateManager::UpdateState(float DeltaTime)
{
	if (!OwningCharacter)
	{
		return;
	}

	// Store previous state for delta detection
	PreviousTags = Tags;

	// Auto-sync various systems
	if (bAutoSyncMovementTags)
	{
		AutoSyncMovementMode();
		AutoSyncGait();
		AutoSyncStance();
	}

	if (bAutoSyncRotationTags)
	{
		AutoSyncRotationMode();
	}

	if (bAutoSyncStatusTags)
	{
		AutoSyncStatus();
	}

	// Detect changes and broadcast events
	DetectAndBroadcastChanges();
}

//~=============================================================================
// Tag Management API
//~=============================================================================

void UFVCharacterStateManager::AddTag(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}

	AddTagInternal(Tag, true);
}

void UFVCharacterStateManager::AddTags(const FGameplayTagContainer& InTags)
{
	for (const FGameplayTag& Tag : InTags)
	{
		AddTagInternal(Tag, false);
	}
	OnTagsChanged.Broadcast(Tags);
}

void UFVCharacterStateManager::RemoveTag(const FGameplayTag& Tag)
{
	RemoveTagInternal(Tag, true);
}

void UFVCharacterStateManager::RemoveTags(const FGameplayTagContainer& InTags)
{
	for (const FGameplayTag& Tag : InTags)
	{
		RemoveTagInternal(Tag, false);
	}
	OnTagsChanged.Broadcast(Tags);
}

bool UFVCharacterStateManager::HasTag(const FGameplayTag& Tag) const
{
	return Tags.HasTag(Tag);
}

bool UFVCharacterStateManager::HasAllTags(const FGameplayTagContainer& InTags) const
{
	return Tags.HasAll(InTags);
}

bool UFVCharacterStateManager::HasAnyTags(const FGameplayTagContainer& InTags) const
{
	return Tags.HasAny(InTags);
}

bool UFVCharacterStateManager::MatchesTagQuery(const FGameplayTagQuery& Query) const
{
	return Query.Matches(Tags);
}

FGameplayTagContainer UFVCharacterStateManager::GetTagsByParent(const FGameplayTag& ParentTag) const
{
	FGameplayTagContainer MatchingTags;
	for (const FGameplayTag& Tag : Tags)
	{
		if (Tag.MatchesTag(ParentTag))
		{
			MatchingTags.AddTag(Tag);
		}
	}
	return MatchingTags;
}

int32 UFVCharacterStateManager::GetTagCount(const FGameplayTag& Tag) const
{
	const int32* Count = TagCounts.Find(Tag);
	return Count ? *Count : (Tags.HasTag(Tag) ? 1 : 0);
}

//~=============================================================================
// Convenience Queries
//~=============================================================================

bool UFVCharacterStateManager::IsInMovementMode(const FGameplayTag& ModeTag) const
{
	return Tags.HasTag(ModeTag);
}

FGameplayTag UFVCharacterStateManager::GetActiveGait() const
{
	// Priority order: Sprinting > Jogging > Walking
	if (Tags.HasTag(FVCharacterTags::Movement_Gait_Sprinting))
	{
		return FVCharacterTags::Movement_Gait_Sprinting;
	}
	if (Tags.HasTag(FVCharacterTags::Movement_Gait_Jogging))
	{
		return FVCharacterTags::Movement_Gait_Jogging;
	}
	return FVCharacterTags::Movement_Gait_Walking;
}

FGameplayTag UFVCharacterStateManager::GetActiveStance() const
{
	if (Tags.HasTag(FVCharacterTags::Movement_Stance_Crouching))
	{
		return FVCharacterTags::Movement_Stance_Crouching;
	}
	// Add more stances here as needed (Prone, etc.)
	return FGameplayTag(); // Standing is absence of stance tags
}

bool UFVCharacterStateManager::HasMovementRestrictions() const
{
	const FFVMovementTags& MovementTags = FFVMovementTags::Get();
	return Tags.HasTag(MovementTags.Movement_Restriction);
}

bool UFVCharacterStateManager::IsActionBlocked(const FGameplayTag& ActionTag) const
{
	// Check for specific restriction tag
	// e.g., Movement.Action.Jump blocked by Movement.Restriction.NoJump
	FString ActionStr = ActionTag.ToString();
	ActionStr.ReplaceInline(TEXT("Movement.Action."), TEXT("Movement.Restriction.No"));
	
	FGameplayTag RestrictionTag = FGameplayTag::RequestGameplayTag(FName(*ActionStr), false);
	return RestrictionTag.IsValid() && Tags.HasTag(RestrictionTag);
}

//~=============================================================================
// Auto-Sync Functions
//~=============================================================================

void UFVCharacterStateManager::AutoSyncMovementMode()
{
	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Clear old movement mode tags
	RemoveTagInternal(FVCharacterTags::Movement_Mode_OnGround, false);
	RemoveTagInternal(FVCharacterTags::Movement_Mode_InAir, false);

	// Set current movement mode tag
	EMovementMode CurrentMode = MovementComp->MovementMode;
	if (FVCharacterTags::MovementModeTagMap.Contains(CurrentMode))
	{
		AddTagInternal(FVCharacterTags::MovementModeTagMap[CurrentMode], false);
	}
}

void UFVCharacterStateManager::AutoSyncGait()
{
	// Gait is intent-based, managed by character actions
	// Ensure at least one gait is always active
	if (!Tags.HasTag(FVCharacterTags::Movement_Gait_Sprinting) &&
		!Tags.HasTag(FVCharacterTags::Movement_Gait_Jogging) &&
		!Tags.HasTag(FVCharacterTags::Movement_Gait_Walking))
	{
		AddTagInternal(FVCharacterTags::Movement_Gait_Walking, false);
	}
}

void UFVCharacterStateManager::AutoSyncStance()
{
	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Sync crouching state
	if (MovementComp->IsCrouching())
	{
		if (!Tags.HasTag(FVCharacterTags::Movement_Stance_Crouching))
		{
			AddTagInternal(FVCharacterTags::Movement_Stance_Crouching, false);
		}
	}
	else
	{
		if (Tags.HasTag(FVCharacterTags::Movement_Stance_Crouching))
		{
			RemoveTagInternal(FVCharacterTags::Movement_Stance_Crouching, false);
		}
	}
}

void UFVCharacterStateManager::AutoSyncRotationMode()
{
	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Rotation mode is part of aim intent, handled by character actions
	// This is a placeholder for more complex rotation logic
}

void UFVCharacterStateManager::AutoSyncStatus()
{
	// Sync idle/moving status
	float CurrentSpeed = OwningCharacter->GetGroundSpeed();
	
	if (CurrentSpeed > KINDA_SMALL_NUMBER)
	{
		RemoveTagInternal(FVCharacterTags::Movement_Status_Idle, false);
	}
	else
	{
		if (!Tags.HasTag(FVCharacterTags::Movement_Status_Idle))
		{
			AddTagInternal(FVCharacterTags::Movement_Status_Idle, false);
		}
	}
}

void UFVCharacterStateManager::DetectAndBroadcastChanges()
{
	// Check for any differences
	if (!Tags.HasAllExact(PreviousTags) || !PreviousTags.HasAllExact(Tags))
	{
		OnTagsChanged.Broadcast(Tags);
	}
}

//~=============================================================================
// Internal Functions
//~=============================================================================

void UFVCharacterStateManager::AddTagInternal(const FGameplayTag& Tag, bool bBroadcast)
{
	if (!Tag.IsValid())
	{
		return;
	}

	bool bWasAdded = false;
	if (!Tags.HasTagExact(Tag))
	{
		Tags.AddTag(Tag);
		bWasAdded = true;

		// Update tag count
		int32& Count = TagCounts.FindOrAdd(Tag, 0);
		Count++;
	}

	if (bBroadcast && bWasAdded)
	{
		OnTagChanged.Broadcast(Tag, true);
		OnTagsChanged.Broadcast(Tags);
	}
}

void UFVCharacterStateManager::RemoveTagInternal(const FGameplayTag& Tag, bool bBroadcast)
{
	bool bWasRemoved = false;
	if (Tags.HasTagExact(Tag))
	{
		Tags.RemoveTag(Tag);
		bWasRemoved = true;

		// Update tag count
		int32* Count = TagCounts.Find(Tag);
		if (Count && *Count > 0)
		{
			(*Count)--;
			if (*Count == 0)
			{
				TagCounts.Remove(Tag);
			}
		}
	}

	if (bBroadcast && bWasRemoved)
	{
		OnTagChanged.Broadcast(Tag, false);
		OnTagsChanged.Broadcast(Tags);
	}
}

