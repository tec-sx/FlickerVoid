// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Nodes/FlowNode.h"
#include "FVFlowNode_SetInteractionState.generated.h"


UCLASS(NotBlueprintable, meta = (DisplayName = "Set Interaction State"))
class FLICKERVOIDGAMEPLAY_API UFVFlowNode_SetInteractionState : public UFlowNode
{
	GENERATED_BODY()
	
public:	
	UFVFlowNode_SetInteractionState();
	
#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
	
protected:
	UPROPERTY(EditAnywhere, Category = "ObservedComponent")
	FGameplayTagContainer IdentityTags;
	
	virtual void ExecuteInput(const FName& PinName) override;

};
