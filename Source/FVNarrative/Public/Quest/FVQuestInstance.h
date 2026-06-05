// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "FVQuestObjective.h"

#include "FVQuestInstance.generated.h"

class UFVQuestDefinition;

/**
 * FFVObjectiveProgress
 * Runtime progress data for a single objective
 */
USTRUCT(BlueprintType)
struct FFVObjectiveProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	FName ObjectiveId;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	EFVObjectiveState State = EFVObjectiveState::Locked;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	float TimeStarted = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	float TimeCompleted = 0.0f;

	/** Custom data for objective-specific tracking */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	TMap<FName, FString> CustomData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, UFVQuestInstance*, Quest, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveStateChanged, UFVQuestInstance*, Quest, FName, ObjectiveId);

/**
 * UFVQuestInstance
 * 
 * Runtime instance of a quest. Tracks player progress through a quest definition.
 * Created and managed by the Quest Subsystem.
 */
UCLASS(BlueprintType)
class FLICKERVOIDNARRATIVE_API UFVQuestInstance : public UObject
{
	GENERATED_BODY()

public:
	UFVQuestInstance();

	// ========================================================================
	// INITIALIZATION
	// ========================================================================

	/** Initialize from a quest definition */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void InitializeFromDefinition(const UFVQuestDefinition* Definition);

	// ========================================================================
	// STATE
	// ========================================================================

	/** The quest definition this is an instance of */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TObjectPtr<const UFVQuestDefinition> QuestDefinition;

	/** Current quest state */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FGameplayTag CurrentState;

	/** Progress for each objective */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TArray<FFVObjectiveProgress> ObjectiveProgress;

	/** Time when quest was started (game time) */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	float TimeStarted = 0.0f;

	/** Time when quest was completed/failed (game time) */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	float TimeEnded = 0.0f;

	/** Tracked as active quest in journal? */
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	bool bIsTracked = false;

	// ========================================================================
	// EVENTS
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveStateChanged OnObjectiveStateChanged;

	// ========================================================================
	// METHODS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void StartQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AbandonQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateObjectiveProgress(FName ObjectiveId, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteObjective(FName ObjectiveId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailObjective(FName ObjectiveId);

	/** Get objective progress - returns a copy of the progress data (Blueprint safe) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	FFVObjectiveProgress GetObjectiveProgressCopy(FName ObjectiveId) const;

	/** Get objective progress pointer - internal use only */
	FFVObjectiveProgress* GetObjectiveProgress(FName ObjectiveId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsObjectiveActive(FName ObjectiveId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsObjectiveComplete(FName ObjectiveId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<FName> GetActiveObjectiveIds() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	float GetOverallProgress() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsComplete() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsFailed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsActive() const;

	/** Store custom data for serialization */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetCustomData(FName Key, const FString& Value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	FString GetCustomData(FName Key) const;

private:
	void SetState(FGameplayTag NewState);
	void UpdateObjectiveStates();
	void CheckQuestCompletion();
	void ApplyConsequences(const struct FFVQuestConsequence& Consequences);

	UPROPERTY()
	TMap<FName, FString> CustomData;
};