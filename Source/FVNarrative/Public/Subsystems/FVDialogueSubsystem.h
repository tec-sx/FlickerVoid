// Fill out your copyright notice in the Description page of Project Settings
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Dialogue/FVDialogueTypes.h"

#include "FVDialogueSubsystem.generated.h"

class UFVDialogueQuery;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueLineStarted, const FFVDialogueLine&, Line, const FFVDialogueContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueLineEnded, const FFVDialogueLine&, Line, bool, bWasSkipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueResponsesAvailable, const TArray<FFVDialogueResponse>&, Responses);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueResponseSelected, const FFVDialogueResponse&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationEnded);

/**
 * FFVActiveConversation
 * Tracks the state of an active conversation
 */
USTRUCT(BlueprintType)
struct FFVActiveConversation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Conversation")
	FName ConversationId;

	UPROPERTY(BlueprintReadOnly, Category = "Conversation")
	FName CurrentSignal;

	UPROPERTY(BlueprintReadOnly, Category = "Conversation")
	FFVDialogueContext Context;

	UPROPERTY(BlueprintReadOnly, Category = "Conversation")
	TArray<FFVDialogueLine> History;

	UPROPERTY(BlueprintReadOnly, Category = "Conversation")
	bool bWaitingForResponse = false;

	UPROPERTY(BlueprintReadOnly, Category = "Conversation")
	TArray<FFVDialogueResponse> AvailableResponses;
};

/**
 * UFVDialogueSubsystem
 * 
 * Central manager for dialogue playback and conversation flow.
 * Integrates with the query system to find appropriate lines.
 */
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVDialogueSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// CONFIGURATION
	// ========================================================================

	/** Set the main dialogue lines table */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetDialogueTable(UDataTable* DialogueTable);

	/** Set the conditions table */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetConditionsTable(UDataTable* ConditionsTable);

	/** Set the responses table */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetResponsesTable(UDataTable* ResponsesTable);

	/** Set the speakers table */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetSpeakersTable(UDataTable* SpeakersTable);

	/** Add a dialogue table (e.g., for DLC or location-specific dialogue) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AddDialogueTable(UDataTable* AdditionalTable);

	// ========================================================================
	// SIGNAL SYSTEM (Like DRS)
	// ========================================================================

	/**
	 * Send a dialogue signal (like DRS QueueSignal)
	 * Finds and plays the best matching line for the signal
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool SendSignal(FName SignalName, const FFVDialogueContext& Context);

	/**
	 * Send a signal with auto-generated context from speaker actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool SendSignalFromActor(FName SignalName, AActor* Speaker, AActor* Listener = nullptr);

	// ========================================================================
	// CONVERSATION SYSTEM
	// ========================================================================

	/**
	 * Start a structured conversation
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conversation")
	bool StartConversation(FName ConversationId, FName InitialSignal, const FFVDialogueContext& Context);

	/**
	 * Select a response in the current conversation
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conversation")
	void SelectResponse(int32 ResponseIndex);

	/**
	 * Select a response by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conversation")
	void SelectResponseById(FName ResponseId);

	/**
	 * Skip the current dialogue line
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conversation")
	void SkipCurrentLine();

	/**
	 * End the current conversation
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Conversation")
	void EndConversation();

	/**
	 * Check if a conversation is active
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue|Conversation")
	bool IsInConversation() const;

	/**
	 * Get the current conversation state
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue|Conversation")
	const FFVActiveConversation& GetActiveConversation() const { return ActiveConversation; }

	// ========================================================================
	// USAGE TRACKING
	// ========================================================================

	/** Check if a line is on cooldown */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool IsLineOnCooldown(FName LineId) const;

	/** Get the use count for a line */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	int32 GetLineUseCount(FName LineId) const;

	/** Reset all cooldowns and use counts */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ResetUsageTracking();

	// ========================================================================
	// SPEAKER INFO
	// ========================================================================

	/** Get speaker info by ID */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool GetSpeakerInfo(FName SpeakerId, FFVSpeakerInfo& OutInfo) const;

	// ========================================================================
	// EVENTS
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueLineStarted OnDialogueLineStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueLineEnded OnDialogueLineEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueResponsesAvailable OnDialogueResponsesAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueResponseSelected OnDialogueResponseSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnConversationStarted OnConversationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnConversationEnded OnConversationEnded;

private:
	UPROPERTY()
	TObjectPtr<UFVDialogueQuery> DialogueQuery;

	UPROPERTY()
	TObjectPtr<UDataTable> ResponsesTable;

	UPROPERTY()
	TObjectPtr<UDataTable> SpeakersTable;

	UPROPERTY()
	FFVActiveConversation ActiveConversation;

	UPROPERTY()
	FFVDialogueLine CurrentLine;

	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentAudioComponent;

	// Usage tracking
	TMap<FName, float> LineCooldowns; // LineId -> Cooldown end time (game hours)
	TMap<FName, int32> LineUseCounts; // LineId -> Use count

	FTimerHandle LineTimerHandle;
	bool bIsPlayingLine = false;

	void PlayLine(const FFVDialogueLine& Line, const FFVDialogueContext& Context);
	void OnLineFinished();
	void ApplyLineEffects(const FFVDialogueLine& Line);
	void ApplyEffectString(const FString& EffectString);
	void ProcessNextSignal();
	void UpdateUsageTracking(const FFVDialogueLine& Line);
	bool CheckUsageLimits(const FFVDialogueLine& Line) const;

	float CalculateLineDuration(const FFVDialogueLine& Line) const;
};