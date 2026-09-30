#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Nodes/FlowNode.h"
#include "Yap/YapCondition.h"
#include "FVDialogueBridge.generated.h"

namespace FVDialogueBridge
{
/** Instigator = first local player pawn, Target = current dialogue speaker, WorldContext = the given object. */
FVSTORYSYSTEM_API FFVConditionContext MakeContext(UObject* WorldContext);
}

/** Yap condition that evaluates a FlickerVoid condition set (facts, checks, knowledge, reputation, time...). */
UCLASS(DisplayName = "FV Conditions")
class FVSTORYSYSTEM_API UFVYapCondition : public UYapCondition
{
GENERATED_BODY()

public:
UPROPERTY(EditAnywhere, Category = "Default")
FFVConditionSet Conditions;

virtual bool EvaluateCondition_Implementation() const override;

#if WITH_EDITOR
virtual FText GetTitle_Implementation() const override;
#endif
};

/** Flow node: evaluates a condition set and routes to True/False. */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Branch"))
class FVSTORYSYSTEM_API UFVFlowNode_Branch : public UFlowNode
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

/** Flow node: applies an effect list (set facts, learn, reputation, cinematic...) and continues. */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Apply Effects"))
class FVSTORYSYSTEM_API UFVFlowNode_ApplyEffects : public UFlowNode
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