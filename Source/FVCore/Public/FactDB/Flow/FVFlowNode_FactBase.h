// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "NativeGameplayTags.h"
#include "FVFlowNode_FactBase.generated.h"

namespace FlowNodeStyle
{
	FLICKERVOIDCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact);
}

UCLASS(Abstract, NotBlueprintable, HideCategories=("Internal"))
class FLICKERVOIDCORE_API UFVFlowNode_FactBase : public UFlowNode
{
	GENERATED_BODY()
public:
	UFVFlowNode_FactBase();
	
protected:
#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
#endif
	
	UPROPERTY(EditAnywhere, Category = "Fact", meta=(Categories = "Fact"))
	FGameplayTag Fact;
};
