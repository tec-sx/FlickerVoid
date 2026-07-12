#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Dialogue/Conditions/FVDialogueConditionSet.h"
#include "FVDialogueResponseRow.generated.h"

UENUM(BlueprintType)
enum class EFVResponseVisibility : uint8
{
    Visible,
    GreyedWhenFailed,
    HiddenWhenFailed,
};

USTRUCT(BlueprintType)
struct FFVDialogueResponseRow : public FTableRowBase
{
    GENERATED_BODY()

    // ── Content ───────────────────────────────────────────────────────────────

    UPROPERTY(
        EditAnywhere, 
        BlueprintReadWrite, 
        Category="Content", 
        meta=(MultiLine=true))
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
    FGameplayTag IconTag;

    // ── Visibility & Availability ─────────────────────────────────────────────
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Availability")
    EFVResponseVisibility VisibilityRule = EFVResponseVisibility::HiddenWhenFailed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Availability")
    FFVDialogueConditionSet Conditions;

    // ── Flow ──────────────────────────────────────────────────────────────────
    
    UPROPERTY(
        EditAnywhere, 
        BlueprintReadWrite, 
        Category="Flow",
        meta=(
            EditCondition="!bEndsDialogue",
            ToolTip="Fixed line response. Takes priority so set to NAME_None if using NextSignalTag."))
    FName NextLineID = NAME_None;
    
    UPROPERTY(
        EditAnywhere, 
        BlueprintReadWrite, 
        Category="Flow", 
        meta=(
            Categories="Dialogue.Signal",
            EditCondition="!bEndsDialogue",
            ToolTip="Re-runs signal selection. Only use when NextLineID is NAME_None"))
    FGameplayTag NextSignalTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flow")
    bool bEndsDialogue = false;

    // ── Events ────────────────────────────────────────────────────────────────
    
    UPROPERTY(
        EditAnywhere, 
        BlueprintReadWrite, 
        Category="Events", 
        meta=(Categories="Dialogue.Event"))
    FGameplayTag EventOnChoose;

    /**
     * Tags to add to the player's tag container when this response is chosen.
     * Useful for tracking conversation choices as player state.
     * e.g. PlayerState.Dialogue.PromisedHelp.MerchantDeale
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Events")
    FGameplayTagContainer TagsToGrantInstigator;
    
    UPROPERTY(
        EditAnywhere, 
        BlueprintReadWrite, 
        Category="Events",
        meta=(ToolTip="Positive = warmer, negative = colder. 0 = no change."))
    int32 RelationshipDelta = 0;

    UPROPERTY(
        EditAnywhere, 
        BlueprintReadWrite, 
        Category="Events",
        meta=(ToolTip="Positive = more notorious, negative = less notorious. 0 = no change."))
    float NotorietyDelta = 0.f;
};