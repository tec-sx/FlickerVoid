// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_EndDialogue.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "End Dialogue"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_EndDialogue : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_EndDialogue();

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	bool bRestoreCamera = true;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	bool bPersistVariables = true;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	bool bWasAborted = true;
	
	virtual void ExecuteInput(const FName& PinName) override;
};
