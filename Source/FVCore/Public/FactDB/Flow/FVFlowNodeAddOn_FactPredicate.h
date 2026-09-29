// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "FactDB/FVFactTypes.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "FVFlowNodeAddOn_FactPredicate.generated.h"

UCLASS(MinimalApi, NotBlueprintable, meta = (DisplayName = "Fact Predicate"))
class UFVFlowNodeAddOn_FactPredicate : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()
	
public:
	UFVFlowNodeAddOn_FactPredicate();
	virtual bool EvaluatePredicate_Implementation() const override;
	
#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "Fact", meta = (ShowOnlyInnerProperties))
	FFVFactCondition Condition;
};
