#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Dialogue/FVDialogueContext.h"
#include "Dialogue/Data/FVDialogueLineRow.h"
#include "Dialogue/Data/FVDialogueResponseRow.h"
#include "FVDialogueConditionEvaluator.generated.h"

struct FScoredLine
{
    const FFVDialogueLineRow* Row = nullptr;
    FName RowKey;
    float ConditionScore = 0.f;
    int32 Priority = 0;

    float TotalScore() const
    {
        // Priority is a hard tier multiplier — a priority 1 line always beats
        // any priority 0 line, regardless of condition score delta.
        return static_cast<float>(Priority) * 1000.f + ConditionScore;
    }
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FFVEvaluatedResponse
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly)
    FName ResponseID;
    
    UPROPERTY(BlueprintReadOnly)
    FText DisplayText;

    UPROPERTY(BlueprintReadOnly)
    FText TooltipText;

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag IconTag;
    
    UPROPERTY(BlueprintReadOnly)
    bool bIsAvailable = true;

    UPROPERTY(BlueprintReadOnly)
    EFVResponseVisibility VisibilityRule = EFVResponseVisibility::HiddenWhenFailed;
    
    bool ShouldShow() const
    {
        if (bIsAvailable) return true;
        return VisibilityRule == EFVResponseVisibility::GreyedWhenFailed;
    }
};

UCLASS()
class FLICKERVOIDNARRATIVE_API UDialogueConditionEvaluator : public UObject
{
    GENERATED_BODY()

public:

    // ─────────────────────────────────────────────────────────────────────────
    // Primary line selection entry point
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * Given a pre-filtered candidate list (same SignalTag + SpeakerID),
     * evaluate all conditions and return the single best line.
     *
     * Returns nullptr if no line passes hard conditions.
     * Applies weighted random among lines with equal total score.
     *
     * @param Candidates   Pairs of (RowKey, RowPtr) — all same signal+speaker
     * @param Context      Immutable context snapshot
     * @param OutSelectedKey  Set to the winning row's key (for consumption tracking)
     */
    static const FFVDialogueLineRow* SelectBestLine(
        const TArray<TPair<FName, 
        const FFVDialogueLineRow*>>& Candidates,
        const FFVDialogueContext& Context,
        FName& OutSelectedKey);

    // ─────────────────────────────────────────────────────────────────────────
    // Response evaluation
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * Evaluate a set of response row IDs and return display-ready structs
     * for the UI. Respects visibility rules (hidden/greyed/normal).
     *
     * @param ResponseIDs   Row keys into DT_DialogueResponses
     * @param ResponseTable The response data table
     * @param Context       Current context snapshot
     * @param OutResponses  Ordered result — only includes rows with ShouldShow()
     */
    static void EvaluateResponses(
        const TArray<FName>& ResponseIDs,
        const UDataTable* ResponseTable,
        const FFVDialogueContext& Context,
        TArray<FFVEvaluatedResponse>& OutResponses);

private:

    /**
     * Step 1: Hard condition filtering.
     * Removes any lines that:
     *   - Have already been consumed (bConsumeOnPlay history)
     *   - Fail FDialogueConditionSet::EvaluateHard()
     */
    static void FilterLines(
        const TArray<TPair<FName, 
        const FFVDialogueLineRow*>>& Candidates,
        const FFVDialogueContext& Context,
        TArray<FScoredLine>& OutPassing);

    /**
     * Step 2: Soft scoring on passing lines.
     * Fills ConditionScore on each FScoredLine.
     */
    static void ScoreLines(
        TArray<FScoredLine>& Lines,
        const FFVDialogueContext& Context);

    /**
     * Step 3: Select winner.
     * Groups by TotalScore() descending, then weighted random within top tier.
     */
    static const FScoredLine* SelectWinner(const TArray<FScoredLine>& ScoredLines);
};