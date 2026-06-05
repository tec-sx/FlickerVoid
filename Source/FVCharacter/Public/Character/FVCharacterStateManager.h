// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FVCharacterStateManager.generated.h"

class AFVCharacter;

/**
 * UFVCharacterStateSubsystem
 * 
 * Manages character state through gameplay tags with automatic lifecycle management,
 * performance optimization, and extensible queries.
 * 
 * PHILOSOPHY:
 * - Tags ARE the state, not a representation of state
 * - All systems query tags, never cache state elsewhere
 * - Automatic tag synchronization from underlying systems
 * - Tag changes trigger events for reactive systems (animation, abilities, UI)
 * 
 * USAGE:
 * - Character owns this subsystem
 * - All state queries go through this
 * - Movement, combat, stats, conditions - everything is tags
 * - Scriptable in Angelscript/Blueprint
 */
UCLASS(BlueprintType)
class FLICKERVOIDCHARACTER_API UFVCharacterStateManager : public UObject
{
	GENERATED_BODY()

public:
	UFVCharacterStateManager();

	/** Initialize subsystem with owning character */
	void Initialize(AFVCharacter* InOwner);

	/** Update state tags from character systems (called every frame) */
	void UpdateState(float DeltaTime);

	//~=============================================================================
	// Tag Management API
	//~=============================================================================

	/** Add a tag to character state */
	UFUNCTION(BlueprintCallable, Category = "Character State")
	void AddTag(const FGameplayTag& Tag);

	/** Add multiple tags */
	UFUNCTION(BlueprintCallable, Category = "Character State")
	void AddTags(const FGameplayTagContainer& Tags);

	/** Remove a tag from character state */
	UFUNCTION(BlueprintCallable, Category = "Character State")
	void RemoveTag(const FGameplayTag& Tag);

	/** Remove multiple tags */
	UFUNCTION(BlueprintCallable, Category = "Character State")
	void RemoveTags(const FGameplayTagContainer& Tags);

	/** Check if character has a specific tag */
	UFUNCTION(BlueprintPure, Category = "Character State")
	bool HasTag(const FGameplayTag& Tag) const;

	/** Check if character has all of the specified tags */
	UFUNCTION(BlueprintPure, Category = "Character State")
	bool HasAllTags(const FGameplayTagContainer& Tags) const;

	/** Check if character has any of the specified tags */
	UFUNCTION(BlueprintPure, Category = "Character State")
	bool HasAnyTags(const FGameplayTagContainer& Tags) const;

	/** Check if character matches tag query (supports complex AND/OR/NOT) */
	UFUNCTION(BlueprintPure, Category = "Character State")
	bool MatchesTagQuery(const FGameplayTagQuery& Query) const;
	
	/** Get all current tags (returns copy for safety) */
	UFUNCTION(BlueprintPure, Category = "Character|Tags")
	FGameplayTagContainer GetAllTags() const { return Tags; }

	/** Get all current tags by reference (PERFORMANCE - no copy, read-only access) */
	FORCEINLINE FGameplayTagContainer& GetAllTagsRef() { return Tags; }

	/** Get tags matching a parent tag (e.g., all "Movement.Gait.*" tags) */
	UFUNCTION(BlueprintPure, Category = "Character State")
	FGameplayTagContainer GetTagsByParent(const FGameplayTag& ParentTag) const;

	/** Get the count of tags matching a parent (useful for stacking effects) */
	UFUNCTION(BlueprintPure, Category = "Character State")
	int32 GetTagCount(const FGameplayTag& Tag) const;

	//~=============================================================================
	// Convenience Queries (Common Patterns)
	//~=============================================================================

	/** Check if character is in any of the specified movement modes */
	UFUNCTION(BlueprintPure, Category = "Character State|Movement")
	bool IsInMovementMode(const FGameplayTag& ModeTag) const;

	/** Get the active gait tag (Walking/Jogging/Sprinting/etc) */
	UFUNCTION(BlueprintPure, Category = "Character State|Movement")
	FGameplayTag GetActiveGait() const;

	/** Get the active stance tag (Standing/Crouching/Prone/etc) */
	UFUNCTION(BlueprintPure, Category = "Character State|Movement")
	FGameplayTag GetActiveStance() const;

	/** Check if character has any movement restrictions */
	UFUNCTION(BlueprintPure, Category = "Character State|Movement")
	bool HasMovementRestrictions() const;

	/** Check if specific action is blocked by restrictions */
	UFUNCTION(BlueprintPure, Category = "Character State|Movement")
	bool IsActionBlocked(const FGameplayTag& ActionTag) const;

	//~=============================================================================
	// Tag Events (for reactive systems)
	//~=============================================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTagChanged, FGameplayTag, Tag, bool, bAdded);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTagsChanged, FGameplayTagContainer, NewTags);

	/** Fired when a single tag is added or removed */
	UPROPERTY(BlueprintAssignable, Category = "Character State|Events")
	FOnTagChanged OnTagChanged;

	/** Fired when tag container changes (batched) */
	UPROPERTY(BlueprintAssignable, Category = "Character State|Events")
	FOnTagsChanged OnTagsChanged;

	//~=============================================================================
	// Automatic Tag Synchronization
	//~=============================================================================

	/** Should automatically sync movement-related tags from movement component? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State|Auto Sync")
	bool bAutoSyncMovementTags = true;

	/** Should automatically sync rotation mode tags? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State|Auto Sync")
	bool bAutoSyncRotationTags = true;

	/** Should automatically sync status tags (idle, landing, etc)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State|Auto Sync")
	bool bAutoSyncStatusTags = true;

protected:
	/** The owning character */
	UPROPERTY(Transient)
	TObjectPtr<AFVCharacter> OwningCharacter;

	/** All current tags representing character state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer Tags;

	/** Tags from previous frame (for delta detection) */
	FGameplayTagContainer PreviousTags;

	/** Cache of tag counts for stacking effects */
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> TagCounts;

	//~=============================================================================
	// Auto-Sync Internal Functions
	//~=============================================================================

	/** Sync movement mode tags from CMC */
	void AutoSyncMovementMode();

	/** Sync gait tags based on character state */
	void AutoSyncGait();

	/** Sync stance tags from CMC */
	void AutoSyncStance();

	/** Sync rotation mode tags */
	void AutoSyncRotationMode();

	/** Sync transient status tags (landing, starting/stopping, etc) */
	void AutoSyncStatus();

	/** Detect tag changes and fire events */
	void DetectAndBroadcastChanges();

	/** Internal add without events (for auto-sync) */
	void AddTagInternal(const FGameplayTag& Tag, bool bBroadcast = true);

	/** Internal remove without events (for auto-sync) */
	void RemoveTagInternal(const FGameplayTag& Tag, bool bBroadcast = true);

	friend class AFVCharacter;
};

