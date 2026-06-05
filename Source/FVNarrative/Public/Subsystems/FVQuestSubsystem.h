// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"

#include "FVQuestSubsystem.generated.h"

class UFVQuestDefinition;
class UFVQuestInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, UFVQuestInstance*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, UFVQuestInstance*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, UFVQuestInstance*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAvailable, const UFVQuestDefinition*, Definition, bool, bNewlyAvailable);

/**
 * UFVQuestSubsystem
 * 
 * Central manager for all quest-related logic.
 * Handles quest availability, progression, and persistence.
 */
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// QUEST MANAGEMENT
	// ========================================================================

	/** Start a quest by definition */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	UFVQuestInstance* StartQuest(const UFVQuestDefinition* Definition);

	/** Start a quest by asset ID */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	UFVQuestInstance* StartQuestById(FPrimaryAssetId QuestAssetId);

	/** Get an active quest instance */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	UFVQuestInstance* GetQuestInstance(FName QuestId) const;

	/** Get all active quests */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<UFVQuestInstance*> GetActiveQuests() const;

	/** Get all quests of a specific type */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<UFVQuestInstance*> GetQuestsByType(FGameplayTag QuestType) const;

	/** Get the currently tracked quest */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	UFVQuestInstance* GetTrackedQuest() const;

	/** Set the tracked quest */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTrackedQuest(UFVQuestInstance* Quest);

	// ========================================================================
	// AVAILABILITY
	// ========================================================================

	/** Check if a quest is available to start */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsQuestAvailable(const UFVQuestDefinition* Definition) const;

	/** Get all currently available quests */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<UFVQuestDefinition*> GetAvailableQuests() const;

	/** Check all quest prerequisites and update availability */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RefreshQuestAvailability();

	// ========================================================================
	// OBJECTIVE INTERACTION
	// ========================================================================

	/** Called when player talks to an NPC */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void NotifyTalkedToNPC(FName NPCId, FName DialogueNodeId = NAME_None);

	/** Called when player obtains an item */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void NotifyItemObtained(FPrimaryAssetId ItemId, int32 Quantity = 1);

	/** Called when player enters a location */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void NotifyLocationEntered(FName LocationId);

	/** Called when a threshold is reached */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void NotifyThresholdReached(FGameplayTag ThresholdType, FName TargetId, float Value);

	/** Called when game time advances */
	UFUNCTION(BlueprintCallable, Category = "Quest|Events")
	void NotifyTimeAdvanced(float GameHours);

	// ========================================================================
	// COMPLETION TRACKING
	// ========================================================================

	/** Check if a quest has been completed (current or past) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool HasCompletedQuest(FName QuestId) const;

	/** Check if a quest has been failed */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool HasFailedQuest(FName QuestId) const;

	/** Get all completed quest IDs */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<FName> GetCompletedQuestIds() const;

	// ========================================================================
	// EVENTS
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestAvailable OnQuestAvailable;

	// ========================================================================
	// SERIALIZATION
	// ========================================================================

	/** Get save data for all quests */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	TArray<uint8> SerializeQuestData() const;

	/** Restore quests from save data */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void DeserializeQuestData(const TArray<uint8>& Data);

private:
	UPROPERTY()
	TMap<FName, TObjectPtr<UFVQuestInstance>> ActiveQuests;

	UPROPERTY()
	TSet<FName> CompletedQuestIds;

	UPROPERTY()
	TSet<FName> FailedQuestIds;

	UPROPERTY()
	TWeakObjectPtr<UFVQuestInstance> TrackedQuest;

	void OnQuestStateChanged(UFVQuestInstance* Quest, FGameplayTag NewState);
};