#pragma once

#include "CoreMinimal.h"
#include "FVDialogueConditionTypes.h"
#include "FVDialogueConditionSet.generated.h"

USTRUCT(BlueprintType)
struct FFVDialogueConditionSet
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FFVTagRequirementCondition> TagRequirements;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFVRelationshipCondition Relationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFVNotorietyCondition Notoriety;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FFVFactionStandingCondition> FactionConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FFVQuestStateCondition> QuestConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FFVLineConsumedCondition> LineHistory;
    
    bool Evaluate(const FFVDialogueContext& Context) const
    {
        for (const FFVTagRequirementCondition& TagReq : TagRequirements)
        {
            if (!TagReq.Evaluate(Context)) return false;
        }

        if (!Relationship.bSoftCondition && !Relationship.Evaluate(Context)) return false;

        if (!Notoriety.bSoftCondition && !Notoriety.Evaluate(Context)) return false;

        for (const FFVFactionStandingCondition& Faction : FactionConditions)
        {
            if (!Faction.Evaluate(Context)) return false;
        }

        for (const FFVQuestStateCondition& Quest : QuestConditions)
        {
            if (!Quest.Evaluate(Context)) return false;
        }

        for (const FFVLineConsumedCondition& History : LineHistory)
        {
            if (!History.Evaluate(Context)) return false;
        }

        return true;
    }
    
    float Score(const FFVDialogueContext& Context) const
    {
        float Total = 0.f;

        for (const FFVTagRequirementCondition& TagReq : TagRequirements)
        {
            Total += TagReq.Score(Context);
        }

        if (Relationship.IsActive())
        {
            Total += Relationship.Score(Context);
        }

        if (Notoriety.IsActive())
        {
            Total += Notoriety.Score(Context);
        }

        for (const FFVFactionStandingCondition& Faction : FactionConditions)
        {
            Total += Faction.Score(Context);
        }

        for (const FFVQuestStateCondition& Quest : QuestConditions)
        {
            Total += Quest.Score(Context);
        }

        for (const FFVLineConsumedCondition& History : LineHistory)
        {
            Total += History.Score(Context);
        }

        return Total;
    }

    bool IsEmpty() const
    {
        return TagRequirements.IsEmpty()
            && !Relationship.IsActive()
            && !Notoriety.IsActive()
            && FactionConditions.IsEmpty()
            && QuestConditions.IsEmpty()
            && LineHistory.IsEmpty();
    }
};