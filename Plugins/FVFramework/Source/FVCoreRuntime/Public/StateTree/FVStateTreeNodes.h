#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "StateTreeConditionBase.h"
#include "StateTreeTaskBase.h"
#include "FVStateTreeNodes.generated.h"

USTRUCT()
struct FVCORERUNTIME_API FFVSTConditionInstanceData
{
	GENERATED_BODY()

	/** Usually bound to the StateTree context actor. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> Instigator;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (Optional))
	TObjectPtr<AActor> Target;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FFVConditionSet Conditions;
};

/** StateTree condition evaluating a FlickerVoid condition set. */
USTRUCT(Category = "FlickerVoid", meta = (DisplayName = "FV Conditions"))
struct FVCORERUNTIME_API FFVSTCondition_Conditions : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FFVSTConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct FVCORERUNTIME_API FFVSTApplyEffectsInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> Instigator;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (Optional))
	TObjectPtr<AActor> Target;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FFVEffectList OnEnter;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FFVEffectList OnExit;
};

/** StateTree task applying FlickerVoid effects on state enter/exit. Succeeds immediately if OnExit is empty. */
USTRUCT(Category = "FlickerVoid", meta = (DisplayName = "FV Apply Effects"))
struct FVCORERUNTIME_API FFVSTTask_ApplyEffects : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FFVSTApplyEffectsInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};