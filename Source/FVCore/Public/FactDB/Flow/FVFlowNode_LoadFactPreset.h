// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_LoadFactPreset.generated.h"

class UFVFactPreset;

UCLASS(NotBlueprintable, meta=(DisplayName="Load Fact Presets"))
class FLICKERVOIDCORE_API UFVFlowNode_LoadFactPreset : public UFlowNode
{
	GENERATED_BODY()
public:
	UFVFlowNode_LoadFactPreset();
	
protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
	
	UPROPERTY(EditAnywhere, Category = "Fact")
	TArray<TObjectPtr<UFVFactPreset>> Presets;
	
private:
	void LoadFactPreset(const UObject* WorldContextObject, const UFVFactPreset* Preset);
};
