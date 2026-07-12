// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVDialogueTaskInstanceData.h"
#include "StateTreeTaskBase.h"
#include "FVSTT_DialoguePlayLine.generated.h"

USTRUCT(Category = "Flicker Void",
		meta = (DisplayName = "Play Dialogue Line",
		Description = "StateTree task to play a single dialogue line."))
struct FFVSTT_DialoguePlayLine : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FFVDialogueTaskInstanceData;
    
	UPROPERTY(EditAnywhere, Category="Dialogue", meta=(ClampMin="0.5"))
	float DefaultLineDuration = 3.f;

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
    
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
};
