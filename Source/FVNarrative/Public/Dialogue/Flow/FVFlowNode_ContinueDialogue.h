// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_ContinueDialogue.generated.h"


UCLASS(NotBlueprintable, meta = (DisplayName = "Continue Dialogue"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_ContinueDialogue : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_ContinueDialogue();
	
	virtual void ExecuteInput(const FName& PinName) override;
};
