// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Nodes/FlowNode.h"
#include "FVFlowNode_SetInteractionState.generated.h"


UCLASS()
class FLICKERVOIDCORE_API UFVFlowNode_SetInteractionState : public UFlowNode
{
	GENERATED_BODY()
	
public:	
#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
	
protected:
	UPROPERTY(EditAnywhere, Category = "ObservedComponent")
	FGameplayTagContainer IdentityTags;
	
	virtual void ExecuteInput(const FName& PinName) override;

};
