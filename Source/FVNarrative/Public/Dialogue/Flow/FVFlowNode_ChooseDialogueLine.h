// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_ChooseDialogueLine.generated.h"


UCLASS(NotBlueprintable, meta = (DisplayName = "Choose Dialogue Line"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_ChooseDialogueLine : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_ChooseDialogueLine();
	
	UPROPERTY(EditAnywhere, Category = "Dialogue", meta = (DefaultForInputFlowPin, FlowPinType = Int))
	int32 ChoiceIndex = 0;
	
	virtual void ExecuteInput(const FName& PinName) override;
};
