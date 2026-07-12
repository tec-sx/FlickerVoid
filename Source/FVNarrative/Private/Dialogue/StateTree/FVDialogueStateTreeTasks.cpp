#include "Dialogue/StateTree/FVDialogueStateTreeTasks.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Dialogue/FVDialogueSubsystem.h"
#include "Dialogue/FVDialogueDatabaseSubsystem.h"
#include "Dialogue/Conditions/FVDialogueConditionEvaluator.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static UFVDialogueSubsystem* GetDialogueSubsystem(const FStateTreeExecutionContext& Ctx)
{
    const UObject* Owner = Ctx.GetOwner();
    if (!Owner) return nullptr;
    const UWorld* World = Owner->GetWorld();
    if (!World) return nullptr;
    return UGameplayStatics::GetGameInstance(World)->GetSubsystem<UFVDialogueSubsystem>();
}

static UFVDialogueDatabaseSubsystem* GetDatabaseSubsystem(const FStateTreeExecutionContext& Ctx)
{
    const UObject* Owner = Ctx.GetOwner();
    if (!Owner) return nullptr;
    const UWorld* World = Owner->GetWorld();
    if (!World) return nullptr;
    return UGameplayStatics::GetGameInstance(World)->GetSubsystem<UFVDialogueDatabaseSubsystem>();
}

// ─────────────────────────────────────────────────────────────────────────────
// Task 1: FDialogueSelectLineTask
// ─────────────────────────────────────────────────────────────────────────────

EStateTreeRunStatus FFVDialogueSelectLineTask::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    UFVDialogueDatabaseSubsystem* DB = GetDatabaseSubsystem(Context);

    if (!DB)
    {
        UE_LOG(LogTemp, Error, TEXT("SelectLineTask: No UDialogueDatabaseSubsystem found."));
        return EStateTreeRunStatus::Failed;
    }
    
    FGameplayTag QuerySignal = OverrideSignal.IsValid() ? OverrideSignal : Data.Context.Signal;

    // Direct line jump (from NextLineID on a previous line)
    if (OverrideLineID != NAME_None)
    {
        const FFVDialogueLineRow* Row = DB->FindLinePtr(OverrideLineID);
        if (!Row)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("SelectLineTask: DirectLineID '%s' not found."), *OverrideLineID.ToString());
            return EStateTreeRunStatus::Failed;
        }
        Data.CurrentLineID  = OverrideLineID;
        Data.CurrentLineRow = Row;
        return EStateTreeRunStatus::Succeeded;
    }

    // Full query + evaluation
    TArray<TPair<FName, const FFVDialogueLineRow*>> Candidates;
    const bool bFound = DB->QueryLines(QuerySignal, Data.Context.SpeakerID, Candidates);

    if (!bFound || Candidates.IsEmpty())
    {
        UE_LOG(LogTemp, Log,
            TEXT("SelectLineTask: No lines found for signal '%s' + speaker '%s'."),
            *QuerySignal.ToString(), *Data.Context.SpeakerID.ToString());
        return EStateTreeRunStatus::Failed;
    }

    FName SelectedKey;
    const FFVDialogueLineRow* Selected = 
        UDialogueConditionEvaluator::SelectBestLine(Candidates, Data.Context, SelectedKey);

    if (!Selected)
    {
        UE_LOG(LogTemp, Log,
            TEXT("SelectLineTask: No line passed conditions for signal '%s' + speaker '%s'."),
            *QuerySignal.ToString(), *Data.Context.SpeakerID.ToString());
        return EStateTreeRunStatus::Failed;
    }

    Data.CurrentLineID  = SelectedKey;
    Data.CurrentLineRow = Selected;

    UE_LOG(LogTemp, VeryVerbose,
        TEXT("SelectLineTask: Selected line '%s'."), *SelectedKey.ToString());

    return EStateTreeRunStatus::Succeeded;
}

// ─────────────────────────────────────────────────────────────────────────────
// Task 3: FDialogueWaitForChoiceTask
// ─────────────────────────────────────────────────────────────────────────────

EStateTreeRunStatus FFVDialogueWaitForChoiceTask::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    const FFVDialogueLineRow* Row = Data.CurrentLineRow;

    if (!Row || Row->ResponseOptionIDs.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("WaitForChoiceTask: No response options on current line."));
        return EStateTreeRunStatus::Failed;
    }

    UFVDialogueSubsystem* DS = GetDialogueSubsystem(Context);
    UFVDialogueDatabaseSubsystem* DB = GetDatabaseSubsystem(Context);
    if (!DS || !DB) return EStateTreeRunStatus::Failed;

    // Evaluate all response options
    const UDataTable* ResponseTable = DS->GetResponseTable();
    TArray<FFVEvaluatedResponse> EvaluatedResponses;
    UDialogueConditionEvaluator::EvaluateResponses(
        Row->ResponseOptionIDs,
        ResponseTable,
        Data.Context,
        EvaluatedResponses);

    Data.CurrentResponses = EvaluatedResponses;

    // Present to UI
    DS->BroadcastResponseOptions(EvaluatedResponses);
    Data.ChosenResponseID = NAME_None; // Clear previous choice

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FFVDialogueWaitForChoiceTask::Tick(
    FStateTreeExecutionContext& Context, float DeltaTime) const
{
    UFVDialogueSubsystem* DS = GetDialogueSubsystem(Context);
    if (!DS) return EStateTreeRunStatus::Failed;

    // Check if the player submitted a choice
    const FName ChosenID = DS->GetPendingPlayerChoice();
    if (ChosenID != NAME_None)
    {
        FInstanceDataType& Data = Context.GetInstanceData(*this);
        Data.ChosenResponseID = ChosenID;
        DS->ClearPendingPlayerChoice();
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}

void FFVDialogueWaitForChoiceTask::ExitState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    UFVDialogueSubsystem* DS = GetDialogueSubsystem(Context);
    if (DS)
    {
        DS->BroadcastHideResponses();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Task 4: FDialogueProcessChoiceTask
// ─────────────────────────────────────────────────────────────────────────────

EStateTreeRunStatus FFVDialogueProcessChoiceTask::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    const UFVDialogueSubsystem* DS  = GetDialogueSubsystem(Context);
    const UFVDialogueDatabaseSubsystem* DB = GetDatabaseSubsystem(Context);

    if (!DS || !DB) return EStateTreeRunStatus::Failed;

    const FName ChosenID = Data.ChosenResponseID;
    if (ChosenID == NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessChoiceTask: No choice was made."));
        return EStateTreeRunStatus::Failed;
    }

    const FFVDialogueResponseRow* Row = DB->FindResponsePtr(ChosenID);
    if (!Row)
    {
        UE_LOG(LogTemp, Error, TEXT("ProcessChoiceTask: Response '%s' not found."), *ChosenID.ToString());
        return EStateTreeRunStatus::Failed;
    }

    // Fire event
    if (Row->EventOnChoose.IsValid() && Data.Context.Listener.IsValid())
    {
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                Data.Context.Listener.Get(), 
                Row->EventOnChoose, 
                FGameplayEventData());
    }

    // Apply relationship delta
    if (Row->RelationshipDelta != 0)
    {
        Data.Context.RelationshipValue += Row->RelationshipDelta;
        // TODO: Persist to your relationship subsystem:
        // RelationshipSubsystem->ApplyDelta(Speaker, Listener, Row->RelationshipDelta);
    }

    // Apply notoriety delta
    if (!FMath::IsNearlyZero(Row->NotorietyDelta))
    {
        Data.Context.NotorietyValue = FMath::Clamp(
            Data.Context.NotorietyValue + Row->NotorietyDelta, 0.f, 100.f);
        // TODO: Persist to your stats system
    }

    // Grant tags to instigator
    if (!Row->TagsToGrantInstigator.IsEmpty() && Data.Context.Speaker.IsValid())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Data.Context.Speaker.Get()))
            ASI->GetAbilitySystemComponent()->AddLooseGameplayTags(Row->TagsToGrantInstigator);
    }

    // Ends dialogue immediately
    if (Row->bEndsDialogue)
    {
        Data.bSessionEnded = true;
        return EStateTreeRunStatus::Failed; // Failed → triggers End state transition
    }

    // Set next line for SelectLineTask to pick up
    if (Row->NextLineID != NAME_None)
    {
        Data.CurrentLineID  = Row->NextLineID;
        Data.CurrentLineRow = DB->FindLinePtr(Row->NextLineID);
    }
    else if (Row->NextSignalTag.IsValid())
    {
        // Re-trigger signal selection; clear current line so SelectLine runs fresh
        Data.Context.Signal = Row->NextSignalTag;
        Data.CurrentLineID     = NAME_None;
        Data.CurrentLineRow    = nullptr;
    }
    else
    {
        // No next target — end the dialogue
        Data.bSessionEnded = true;
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Succeeded;
}

// ─────────────────────────────────────────────────────────────────────────────
// Task 5: FDialogueEndTask
// ─────────────────────────────────────────────────────────────────────────────

EStateTreeRunStatus FFVDialogueEndTask::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    const FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (UFVDialogueSubsystem* DS = GetDialogueSubsystem(Context))
    {
        // Persist consumed lines to save game
        DS->PersistConsumedLines(Data.SessionConsumedLines);

        // Notify subsystem session ended (will clean up, hide UI, etc.)
        DS->NotifySessionEnded(Data.Context);

        // Hide dialogue UI
        DS->BroadcastSessionEnded();
    }

    return EStateTreeRunStatus::Succeeded;
}