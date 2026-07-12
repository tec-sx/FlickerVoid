#include "Dialogue/StateTree/FVSTT_DialoguePlayLine.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "StateTreeExecutionContext.h"
#include "Dialogue/FVDialogueSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

static UFVDialogueSubsystem* GetDialogueSubsystem(const FStateTreeExecutionContext& Ctx)
{
    const UObject* Owner = Ctx.GetOwner();
    if (!Owner) return nullptr;
    const UWorld* World = Owner->GetWorld();
    if (!World) return nullptr;
    return UGameplayStatics::GetGameInstance(World)->GetSubsystem<UFVDialogueSubsystem>();
}

EStateTreeRunStatus FFVSTT_DialoguePlayLine::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    const FFVDialogueLineRow* Row = Data.CurrentLineRow;

    if (!Row)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayLineTask: CurrentLineRow is null."));
        return EStateTreeRunStatus::Failed;
    }

    UFVDialogueSubsystem* DS = GetDialogueSubsystem(Context);
    if (!DS)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayLineTask: No UDialogueSubsystem found."));
        return EStateTreeRunStatus::Failed;
    }
    
    Data.Context.SessionPlayedLineIDs.Add(Data.CurrentLineID);
    
    if (Row->bConsumeOnPlay)
    {  
        Data.SessionConsumedLines.Add(Data.CurrentLineID);
    }

    // Broadcast to UI/audio via subsystem delegate
    // The subsystem notifies all listeners (widget controller, audio manager, etc.)
    DS->BroadcastLineStarted(Data.CurrentLineID, *Row, Data.Context);

    // Fire EventOnPlay if set
    if (Row->EventOnPlay.IsValid() && Data.Context.Listener.IsValid())
    {
        // Use your event dispatch mechanism here.
        // Example with GAS:
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            Data.Context.Listener.Get(), 
            Row->EventOnPlay, 
            FGameplayEventData());
    }

    return EStateTreeRunStatus::Running; // Wait for completion signal
}

EStateTreeRunStatus FFVSTT_DialoguePlayLine::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
    const UFVDialogueSubsystem* DS = GetDialogueSubsystem(Context);
    if (!DS) return EStateTreeRunStatus::Failed;

    // Subsystem sets a flag when the line is "done" (audio complete or UI dismissed)
    if (DS->IsCurrentLineComplete())
    {
        const FInstanceDataType& Data = Context.GetInstanceData(*this);
        const FFVDialogueLineRow* Row = Data.CurrentLineRow;

        // Fire EventOnComplete
        if (Row && Row->EventOnComplete.IsValid() && Data.Context.Listener.IsValid())
        {
            UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                Data.Context.Listener.Get(), 
                Row->EventOnPlay, 
                FGameplayEventData());
        }

        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}

void FFVSTT_DialoguePlayLine::ExitState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    // Nothing to clean up here — subsystem state is cleared when next line starts
}