#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Nodes/FlowNode.h"
#include "FVCoreFlowNodes.generated.h"

/** Evaluates a condition set and routes to True/False. */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Branch"))
class FVCOREFLOW_API UFVFlowNode_Branch : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_Branch();

	UPROPERTY(EditAnywhere, Category = "Branch")
	FFVConditionSet Conditions;

	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif
};

/** Applies an effect list (facts, knowledge, reputation, items, cinematics...) and continues. */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Apply Effects"))
class FVCOREFLOW_API UFVFlowNode_ApplyEffects : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_ApplyEffects();

	UPROPERTY(EditAnywhere, Category = "Effects")
	FFVEffectList Effects;

	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif
};

/** Holds the signal until the conditions pass. Re-evaluated whenever a fact changes. */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Wait For Conditions"))
class FVCOREFLOW_API UFVFlowNode_WaitForConditions : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_WaitForConditions();

	UPROPERTY(EditAnywhere, Category = "Wait")
	FFVConditionSet Conditions;

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif

private:
	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);
	bool TryFinish();

	FDelegateHandle FactHandle;
};

/** Predicate add-on evaluating any FlickerVoid condition set. */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Conditions Predicate"))
class FVCOREFLOW_API UFVFlowNodeAddOn_ConditionPredicate : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	UFVFlowNodeAddOn_ConditionPredicate();

	UPROPERTY(EditAnywhere, Category = "Conditions")
	FFVConditionSet Conditions;

	virtual bool EvaluatePredicate_Implementation() const override;
};
