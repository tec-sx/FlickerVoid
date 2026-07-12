#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Dialogue/FVDialogueContext.h"
#include "FVDialogueConditionTypes.generated.h"

UENUM(BlueprintType)
enum class EFVTagConditionTarget : uint8
{
    World      = 0,
    Instigator = 1,
    Target     = 2,
};

USTRUCT(BlueprintType)
struct FFVTagRequirementCondition
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFVTagConditionTarget Target = EFVTagConditionTarget::Target;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer RequiredTags;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer BlockedTags;

    bool Evaluate(const FFVDialogueContext& Ctx) const
    {
        const FGameplayTagContainer& Container = ResolveContainer(Ctx);
        if (RequiredTags.Num() > 0 && !Container.HasAll(RequiredTags)) return false;
        if (BlockedTags.Num()  > 0 &&  Container.HasAny(BlockedTags))  return false;
        return true;
    }
    
    float Score(const FFVDialogueContext& Ctx) const
    {
        if (!Evaluate(Ctx)) return 0.f;
        return (RequiredTags.Num() + BlockedTags.Num()) * 10.f;
    }

private:
    const FGameplayTagContainer& ResolveContainer(const FFVDialogueContext& Ctx) const
    {
        switch (Target)
        {
        case EFVTagConditionTarget::World:       return Ctx.WorldStateTags;
        case EFVTagConditionTarget::Instigator:  return Ctx.InstigatorTags;
        case EFVTagConditionTarget::Target:      return Ctx.TargetTags;
        default:                                 return Ctx.TargetTags;
        }
    }
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FFVRelationshipCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
    int32 Min = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
    int32 Max = INT32_MAX;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSoftCondition = false;

    bool IsActive() const { return Max >= 0; }

    bool Evaluate(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive()) return true;
        return Ctx.RelationshipValue >= Min && Ctx.RelationshipValue <= Max;
    }

    float Score(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive() || !Evaluate(Ctx)) return 0.f;
        // Tighter range = more specific = higher score
        const float RangeSize = FMath::Max(1.f, static_cast<float>(Max - Min));
        return 15.f + (100.f / RangeSize);
    }
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FFVNotorietyCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="100.0"))
    float Min = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="100.0"))
    float Max = 100.f;

    bool bSoftCondition = false;

    bool IsActive() const { return Max > 0.f || Min > 0.f; }

    bool Evaluate(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive()) return true;
        return Ctx.NotorietyValue >= Min && Ctx.NotorietyValue <= Max;
    }

    float Score(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive() || !Evaluate(Ctx)) return 0.f;
        const float RangeSize = FMath::Max(1.f, Max - Min);
        return 10.f + (100.f / RangeSize);
    }
};

// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EFVFactionStandingOp : uint8
{
    AtLeast,
    AtMost,
    Exactly,
};

USTRUCT(BlueprintType)
struct FFVFactionStandingCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag FactionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFVFactionStanding RequiredStanding = EFVFactionStanding::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFVFactionStandingOp Operator = EFVFactionStandingOp::AtLeast;

    bool IsActive() const { return FactionTag.IsValid(); }

    bool Evaluate(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive()) return true;
        const EFVFactionStanding Actual = Ctx.GetFactionStanding(FactionTag);
        switch (Operator)
        {
            case EFVFactionStandingOp::AtLeast: return Actual >= RequiredStanding;
            case EFVFactionStandingOp::AtMost:  return Actual <= RequiredStanding;
            case EFVFactionStandingOp::Exactly: return Actual == RequiredStanding;
        }
        return false;
    }

    float Score(const FFVDialogueContext& Ctx) const
    {
        return (IsActive() && Evaluate(Ctx)) ? 12.f : 0.f;
    }
};


// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EFVQuestConditionOp : uint8
{
    IsActive,
    IsNotActive,
    IsComplete,
};

USTRUCT(BlueprintType)
struct FFVQuestStateCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories = "Quest.State"))
    FGameplayTag QuestTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFVQuestConditionOp Operator = EFVQuestConditionOp::IsActive;

    bool IsActive() const { return QuestTag.IsValid(); }

    bool Evaluate(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive()) return true;
        switch (Operator)
        {
            case EFVQuestConditionOp::IsActive:
                return Ctx.ActiveQuestTags.HasTag(QuestTag);
            case EFVQuestConditionOp::IsNotActive:
                return !Ctx.ActiveQuestTags.HasTag(QuestTag);
            case EFVQuestConditionOp::IsComplete:
            {
                // Convention: complete state is tagged as Quest.State.Completed
                // Check if any child tag matching .Completed is present
                return Ctx.ActiveQuestTags.HasTag(QuestTag) &&
                       Ctx.ActiveQuestTags.HasTagExact(
                           FGameplayTag::RequestGameplayTag(
                               FName(QuestTag.ToString() + TEXT(".Complete"))));
            }
        }
        return false;
    }
    
    float Score(const FFVDialogueContext& Ctx) const
    {
        return (IsActive() && Evaluate(Ctx)) ? 20.f : 0.f;
    }
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FFVLineConsumedCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ReferencedLineID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequireConsumed = true;

    bool IsActive() const { return ReferencedLineID != NAME_None; }

    bool Evaluate(const FFVDialogueContext& Ctx) const
    {
        if (!IsActive()) return true;
        const bool bConsumed = Ctx.HasConsumed(ReferencedLineID);
        return bRequireConsumed ? bConsumed : !bConsumed;
    }

    float Score(const FFVDialogueContext& Ctx) const
    {
        return (IsActive() && Evaluate(Ctx)) ? 8.f : 0.f;
    }
};