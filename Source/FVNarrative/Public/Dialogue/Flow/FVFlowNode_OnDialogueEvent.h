// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "FVFlowNode_OnDialogueEvent.generated.h"

struct FFVDialogueEventMessage;

UCLASS(NotBlueprintable, meta = (DisplayName = "On Dialogue Event"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_OnDialogueEvent : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_OnDialogueEvent();
	
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FName EventName;

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

protected:
	void OnEventMessage(FGameplayTag Channel, const FFVDialogueEventMessage& Message);

	FGameplayMessageListenerHandle Handle;

	static FName INPIN_In;
	static FName OUTPIN_Out;
	static FName OUTPIN_Event;
};
