#pragma once

#include "CoreMinimal.h"
#include "FVDialogueTaskInstanceData.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "FVDialogueStateTreeTasks.generated.h"

class UDialogueSubsystem;
class UDialogueDatabaseSubsystem;
class UDialogueSession;

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT()
struct FFVDialogueSelectLineTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FFVDialogueTaskInstanceData;

    UPROPERTY(
        EditAnywhere, 
        Category="Dialogue",
        meta=(
            Categories="Dialogue.Signal",
            ToolTip="Override to query a specific signal tag (for branching re-entry)."))
    FGameplayTag OverrideSignal;

    UPROPERTY(
        EditAnywhere, 
        Category="Dialogue",
        meta=(ToolTip="Override to jump directly to a specific line ID. If None, runs full query + evaluation."))
    FName OverrideLineID = NAME_None;

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override
    {
        return EStateTreeRunStatus::Running; // Resolved in EnterState
    }
    
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT()
struct FFVDialogueWaitForChoiceTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FFVDialogueTaskInstanceData;

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

    virtual void ExitState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override; 
    
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT()
struct FFVDialogueProcessChoiceTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FFVDialogueTaskInstanceData;

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override
    {
        return EStateTreeRunStatus::Running;
    }
    
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
};


// ─────────────────────────────────────────────────────────────────────────────

USTRUCT()
struct FFVDialogueEndTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FFVDialogueTaskInstanceData;

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override
    {
        return EStateTreeRunStatus::Succeeded;
    }
    
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
};