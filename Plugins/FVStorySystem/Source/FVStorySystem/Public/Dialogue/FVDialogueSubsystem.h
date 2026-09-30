#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/Interfaces/IYapConversationHandler.h"
#include "FVDialogueSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVDialogueEvent);

/** A player choice offered by Yap. */
USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVDialoguePrompt
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FText Text;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FText Title;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FYapPromptHandle Handle;
};

/** The line currently being spoken. */
USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FName SpeakerID;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FText SpeakerName;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") TWeakObjectPtr<AActor> SpeakerActor;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FGameplayTag Mood;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FText Text;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") FText Title;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") float Duration = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue") bool bSkippable = false;
};

/**
 * Yap conversation handler for the game. Tracks the active conversation, line, speaker actor
 * (via Yap character components) and prompts, and exposes them to UI through parameterless events.
 */
UCLASS()
class FVSTORYSYSTEM_API UFVDialogueSubsystem : public UWorldSubsystem, public IYapConversationHandler
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Dialogue", meta = (WorldContext = "WorldContext"))
	static UFVDialogueSubsystem* Get(const UObject* WorldContext);

	/** Actor owning the Yap character component registered for this character id. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Dialogue", meta = (WorldContext = "WorldContext"))
	static AActor* FindSpeakerActor(const UObject* WorldContext, FName SpeakerID);

	static AActor* FindSpeakerActorForContext(const UObject* WorldContext);

UFUNCTION(BlueprintPure, Category = "FlickerVoid|Dialogue") bool IsInConversation() const { return Conversation.IsValid(); }
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Dialogue") const FFVDialogueLine& GetCurrentLine() const { return CurrentLine; }
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Dialogue") const TArray<FFVDialoguePrompt>& GetPrompts() const { return Prompts; }
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Dialogue") AActor* GetCurrentSpeaker() const { return CurrentLine.SpeakerActor.Get(); }

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Dialogue")
	void ChoosePrompt(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Dialogue")
	void Advance();

	UPROPERTY(BlueprintAssignable, Category = "FlickerVoid|Dialogue") FFVDialogueEvent OnConversationStarted;
	UPROPERTY(BlueprintAssignable, Category = "FlickerVoid|Dialogue") FFVDialogueEvent OnLineChanged;
	UPROPERTY(BlueprintAssignable, Category = "FlickerVoid|Dialogue") FFVDialogueEvent OnPromptsReady;
	UPROPERTY(BlueprintAssignable, Category = "FlickerVoid|Dialogue") FFVDialogueEvent OnConversationEnded;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	virtual void OnConversationOpened(FYapData_ConversationOpened Data, FYapConversationHandle Handle) override;
	virtual void OnConversationSpeechBegins(FYapData_SpeechBegins Data, FYapSpeechHandle Handle) override;
	virtual void OnConversationPlayerPromptCreated(FYapData_PlayerPromptCreated Data, FYapPromptHandle Handle) override;
	virtual void OnConversationPlayerPromptsReady(FYapData_PlayerPromptsReady Data) override;
	virtual void OnConversationPlayerPromptChosen(FYapData_PlayerPromptChosen Data, FYapPromptHandle Handle) override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	UFUNCTION()
	void HandleConversationClosed(UObject* Instigator, FYapConversationHandle Handle);

	FYapConversationHandle Conversation;
	FYapSpeechHandle Speech;
	FFVDialogueLine CurrentLine;
	TArray<FFVDialoguePrompt> Prompts;
	bool bRegistered = false;
};
