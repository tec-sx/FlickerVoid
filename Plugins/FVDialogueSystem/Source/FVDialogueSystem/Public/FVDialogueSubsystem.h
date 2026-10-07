#pragma once

#include "CoreMinimal.h"
#include "FVDialogueTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVDialogueSubsystem.generated.h"

class UAudioComponent;
class UFlowAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnConversationEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnDialogueLine, const FFVDialogueLine&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnDialogueChoices, const TArray<FFVDialogueChoice>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnBark, AActor*, Speaker, const FText&, Text, float, Duration);

DECLARE_DELEGATE(FFVDialogueLineFinished);
DECLARE_DELEGATE_OneParam(FFVDialogueChoiceMade, int32 /*Index*/);

/**
 * Runs the active conversation: starts dialogue flows, presents lines and choices to UI,
 * plays voices and notifies participant components. UI binds to the events and calls Advance/Choose.
 */
UCLASS()
class FVDIALOGUESYSTEM_API UFVDialogueSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVDialogueSubsystem* Get(const UObject* WorldContext);

	/** Starts Dialogue as a root flow owned by Owner (usually the NPC). Fails if a conversation is running. */
	UFUNCTION(BlueprintCallable, Category = "FV|Dialogue")
	bool StartConversation(UFlowAsset* Dialogue, AActor* Instigator, AActor* Owner);

	/** Ends the conversation and aborts its flow. */
	UFUNCTION(BlueprintCallable, Category = "FV|Dialogue")
	void EndConversation();

	/** Finishes the current line (if skippable). */
	UFUNCTION(BlueprintCallable, Category = "FV|Dialogue")
	void Advance();

	UFUNCTION(BlueprintCallable, Category = "FV|Dialogue")
	bool Choose(int32 Index);

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	bool IsInConversation() const { return bActive; }

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	AActor* GetInstigator() const { return Instigator.Get(); }

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	AActor* GetOwnerActor() const { return Owner.Get(); }

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	const FFVDialogueLine& GetCurrentLine() const { return CurrentLine; }

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	const TArray<FFVDialogueChoice>& GetChoices() const { return Choices; }

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	bool IsWaitingForChoice() const { return ChoiceMade.IsBound(); }

	/** Participant actor for a speaker: the instigator or owner if they match, otherwise any registered actor. */
	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	AActor* ResolveSpeaker(const UFVCharacterDefinition* Speaker) const;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnConversationEvent OnConversationStarted;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnConversationEvent OnConversationEnded;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnDialogueLine OnLineStarted;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnDialogueLine OnLineFinished;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnDialogueChoices OnChoicesPresented;

	/** Ambient lines outside conversations. */
	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnBark OnBark;

	/** Opens a conversation without starting a flow (graph already running, e.g. started by a Flow component). */
	void BeginConversation(AActor* InInstigator, AActor* InOwner);

	/** Ends without touching the flow; used by End Conversation, which finishes the graph itself. */
	void EndConversationFromFlow();

	void PlayLine(const FFVDialogueLine& Line, FFVDialogueLineFinished OnFinished);
	void PresentChoices(const TArray<FFVDialogueChoice>& InChoices, FFVDialogueChoiceMade OnChosen);

	/** Drops pending callbacks, used when the requesting node is cleaned up. */
	void CancelPending();

	void Bark(AActor* Speaker, const FText& Text, float Duration);

	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void FinishLine();
	void StopVoice();
	void Close(bool bAbortFlow);
	float ComputeDuration(const FFVDialogueLine& Line) const;

	TWeakObjectPtr<AActor> Instigator;
	TWeakObjectPtr<AActor> Owner;

	UPROPERTY(Transient)
	TObjectPtr<UFlowAsset> Dialogue;

	UPROPERTY(Transient)
	FFVDialogueLine CurrentLine;

	UPROPERTY(Transient)
	TArray<FFVDialogueChoice> Choices;

	FFVDialogueLineFinished LineFinished;
	FFVDialogueChoiceMade ChoiceMade;
	FTimerHandle LineTimer;
	TWeakObjectPtr<UAudioComponent> Voice;
	bool bActive = false;
};
