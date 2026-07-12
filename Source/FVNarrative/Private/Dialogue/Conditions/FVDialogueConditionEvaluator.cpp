#include "Dialogue/Conditions/FVDialogueConditionEvaluator.h"
#include "Engine/DataTable.h"

// ─────────────────────────────────────────────────────────────────────────────

const FFVDialogueLineRow* UDialogueConditionEvaluator::SelectBestLine(
    const TArray<TPair<FName, 
    const FFVDialogueLineRow*>>& Candidates,
    const FFVDialogueContext& Context,
    FName& OutSelectedKey)
{
    OutSelectedKey = NAME_None;

    if (Candidates.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueConditionEvaluator: No candidates passed in."));
        return nullptr;
    }
    
    TArray<FScoredLine> PassingLines;
    FilterLines(Candidates, Context, PassingLines);

    if (PassingLines.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue: All %d candidates failed condition filter."), Candidates.Num());
        return nullptr;
    }
    
    ScoreLines(PassingLines, Context);
    
    const FScoredLine* Winner = SelectWinner(PassingLines);
    if (!Winner)
    {
        return nullptr;
    }

    OutSelectedKey = Winner->RowKey;
    return Winner->Row;
}

// ─────────────────────────────────────────────────────────────────────────────

void UDialogueConditionEvaluator::FilterLines(
    const TArray<TPair<FName, 
    const FFVDialogueLineRow*>>& Candidates,
    const FFVDialogueContext& Context,
    TArray<FScoredLine>& OutPassing)
{
    OutPassing.Reserve(Candidates.Num());

    for (const TPair<FName, const FFVDialogueLineRow*>& Pair : Candidates)
    {
        const FName& Key = Pair.Key;
        const FFVDialogueLineRow* Row = Pair.Value;

        if (!Row) continue;
        
        if (!Row->bIsRepeatable && Context.HasConsumed(Key))
        {
            continue;
        }
        
        if (!Row->Conditions.Evaluate(Context))
        {
            continue;
        }

        FScoredLine Scored;
        Scored.Row      = Row;
        Scored.RowKey   = Key;
        Scored.Priority = Row->Priority;
        OutPassing.Add(Scored);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void UDialogueConditionEvaluator::ScoreLines(
    TArray<FScoredLine>& Lines,
    const FFVDialogueContext& Context)
{
    for (FScoredLine& Scored : Lines)
    {
        Scored.ConditionScore = Scored.Row->Conditions.Score(Context);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

const FScoredLine* UDialogueConditionEvaluator::SelectWinner(
    const TArray<FScoredLine>& ScoredLines)
{
    if (ScoredLines.IsEmpty()) return nullptr;
    
    float MaxScore = TNumericLimits<float>::Lowest();
    for (const FScoredLine& Scored : ScoredLines)
    {
        MaxScore = FMath::Max(MaxScore, Scored.TotalScore());
    }
    
    TArray<const FScoredLine*> TopTier;
    float TotalWeight = 0.f;

    for (const FScoredLine& Scored : ScoredLines)
    {
        if (FMath::IsNearlyEqual(Scored.TotalScore(), MaxScore, 0.01f))
        {
            TopTier.Add(&Scored);
            TotalWeight += Scored.Row->Weight;
        }
    }

    if (TopTier.Num() == 1)
    {
        return TopTier[0];
    }
    
    float Roll = FMath::FRandRange(0.f, TotalWeight);
    float Accumulated = 0.f;

    for (const FScoredLine* Candidate : TopTier)
    {
        Accumulated += Candidate->Row->Weight;
        if (Roll <= Accumulated)
        {
            return Candidate;
        }
    }
    
    return TopTier.Last();
}

// ─────────────────────────────────────────────────────────────────────────────

void UDialogueConditionEvaluator::EvaluateResponses(
    const TArray<FName>& ResponseIDs,
    const UDataTable* ResponseTable,
    const FFVDialogueContext& Context,
    TArray<FFVEvaluatedResponse>& OutResponses)
{
    OutResponses.Reset();

    if (!ResponseTable)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueConditionEvaluator::EvaluateResponses — null ResponseTable"));
        return;
    }

    for (const FName& ID : ResponseIDs)
    {
        const FFVDialogueResponseRow* Row = ResponseTable->FindRow<FFVDialogueResponseRow>(ID, TEXT("EvaluateResponses"));
        if (!Row)
        {
            UE_LOG(LogTemp, Warning, TEXT("EvaluateResponses: Response ID '%s' not found in table."), *ID.ToString());
            continue;
        }

        const bool bAvailable = Row->Conditions.IsEmpty() || Row->Conditions.Evaluate(Context);

        FFVEvaluatedResponse Evaluated;
        Evaluated.ResponseID     = ID;
        Evaluated.DisplayText    = Row->Text;
        Evaluated.IconTag        = Row->IconTag;
        Evaluated.bIsAvailable   = bAvailable;
        Evaluated.VisibilityRule = Row->VisibilityRule;

        if (Evaluated.ShouldShow())
        {
            OutResponses.Add(Evaluated);
        }
    }
}