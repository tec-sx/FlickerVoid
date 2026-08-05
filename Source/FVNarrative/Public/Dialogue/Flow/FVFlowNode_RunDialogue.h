// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "SUDSValue.h"
#include "Dialogue/FVDialogueMessageTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "FVFlowNode_RunDialogue.generated.h"

class UFVDialogueSet;
struct FFVDialogueEndedMessage;
struct FFVDialogueSubmitChoiceMessage;
class USUDSDialogue;
struct FGameplayAttribute;
class USUDSScript;

UCLASS(NotBlueprintable, meta = (DisplayName = "Run Dialogue"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_RunDialogue : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_RunDialogue();
	
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TObjectPtr<UFVDialogueSet> DialogueSet;
	
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FName StartLabel = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FGameplayTagContainer IdentityTags;
	
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;
	
private:
	UPROPERTY()
	TObjectPtr<USUDSDialogue> Dialogue;
	
	FGameplayMessageListenerHandle ChoiceListenerHandle;
	FGameplayMessageListenerHandle ContinueListenerHandle;

	static FName INPIN_In;
	static FName OUTPIN_Out;
	static FName OUTPIN_OnSpeakerLine;
	static FName OUTPIN_OnChoices;
	static FName OUTPIN_OnEvent;
	
	void StartDialogue();
	void PushInitialVariables(USUDSDialogue* InDialogue, AActor* Owner, AActor* Player);
	AActor* ResolveOwnerActor() const;
	AActor* ResolvePlayerActor() const;
	
	void BroadcastLine() const;
	void BroadcastChoices() const;
	void BuildChoiceList(TArray<FFVDialogueChoiceMessage>& OutChoices) const;

	void RegisterUIListeners();
	void UnregisterUIListeners();
	
	UFUNCTION() void OnSpeakerLine(USUDSDialogue* InDialogue);
	UFUNCTION() void OnFinished(USUDSDialogue* InDialogue);
	UFUNCTION() void OnEvent(USUDSDialogue* InDialogue, FName EventName, const TArray<FSUDSValue>& Arguments);
	
	void OnSubmitChoice(FGameplayTag Channel, const FFVDialogueSubmitChoiceMessage& Message);
	void OnSubmitContinue(FGameplayTag Channel, const FFVDialogueEndedMessage& Message);
	
#if WITH_EDITOR
public:
	virtual FString GetNodeDescription() const override;
	virtual FString GetStatusString() const override;
protected:
	virtual EDataValidationResult ValidateNode() override;
#endif
};
