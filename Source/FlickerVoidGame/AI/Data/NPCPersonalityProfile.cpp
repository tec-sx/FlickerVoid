// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCPersonalityProfile.h"
#include "Character/FVCharacterAttributeComponent.h"
#include "Attributes/FVCharacterAttributeSet.h"
#include "FVAITags.h"

float UNPCPersonalityProfile::EvaluateRelationship(UFVCharacterAttributeComponent* NPCAttributes, UFVCharacterAttributeComponent* TargetAttributes) const
{
    if (!NPCAttributes || !TargetAttributes)
    {
        return 0.0f;
    }

    float RelationshipScore = BaseRelationship;

    // Apply each relationship weight
    for (const FAttributeWeight& Weight : RelationshipWeights)
    {
        RelationshipScore += ApplyAttributeWeight(Weight, NPCAttributes, TargetAttributes);
    }

    return FMath::Clamp(RelationshipScore, -1.0f, 1.0f);
}

float UNPCPersonalityProfile::ApplyAttributeWeight(
    const FAttributeWeight& WeightRule,
    UFVCharacterAttributeComponent* NPCAttributes,
    UFVCharacterAttributeComponent* TargetAttributes) const
{
    // Get target's attribute value
    float TargetValue = TargetAttributes->GetModifiedAttributeValue(WeightRule.TargetAttribute);

    // Get NPC's personality modifier
    float ModifierValue = 1.0f;
    if (!WeightRule.NPCPersonalityModifier.IsNone())
    {
        ModifierValue = NPCAttributes->GetModifiedAttributeValue(WeightRule.NPCPersonalityModifier);

        if (WeightRule.bInvertModifier)
        {
            ModifierValue = 1.0f - ModifierValue;
        }
    }

    // Apply operation
    float FinalWeight = WeightRule.Weight;
    switch (WeightRule.Operator)
    {
    case EEvaluationOperator::Multiply:
        FinalWeight *= ModifierValue;
        break;
    case EEvaluationOperator::Add:
        FinalWeight += ModifierValue;
        break;
    case EEvaluationOperator::Subtract:
        FinalWeight -= ModifierValue;
        break;
    case EEvaluationOperator::Min:
        FinalWeight = FMath::Min(FinalWeight, ModifierValue);
        break;
    case EEvaluationOperator::Max:
        FinalWeight = FMath::Max(FinalWeight, ModifierValue);
        break;
    }

    return TargetValue * FinalWeight;
}

bool UNPCPersonalityProfile::CheckRecognition(UFVCharacterAttributeComponent* NPCAttributes, UFVCharacterAttributeComponent* TargetAttributes) const
{
    if (!NPCAttributes || !TargetAttributes)
    {
        return false;
    }

    float RecognitionChance = BaseRecognitionChance;

    // Target's notoriety increases recognition
    RecognitionChance += TargetAttributes->GetAttributeSet()->GetNotoriety() * NotorietyRecognitionWeight;

    // NPC's intelligence increases recognition
    RecognitionChance += NPCAttributes->GetModifiedAttributeValue(FName("Intelligence")) * IntelligenceRecognitionWeight;

    // Intoxication reduces recognition
    float Intoxication = NPCAttributes->GetModifiedAttributeValue(FName("Intoxication"));
    RecognitionChance *= (1.0f - Intoxication * 0.5f);

    // Alertness affects recognition
    float Alertness = NPCAttributes->GetModifiedAttributeValue(FName("Alertness"));
    RecognitionChance *= FMath::Lerp(0.5f, 1.0f, Alertness);

    return FMath::FRand() < FMath::Clamp(RecognitionChance, 0.0f, 1.0f);
}

FGameplayTag UNPCPersonalityProfile::DetermineBehavior(float RelationshipScore, UFVCharacterAttributeComponent* NPCAttributes) const
{
    if (!NPCAttributes)
    {
        return FVAITags::Behavior_Decision_Ignore;
    }

    // Sort thresholds by priority
    TArray<FBehaviorThreshold> SortedThresholds = BehaviorThresholds;
    SortedThresholds.Sort([](const FBehaviorThreshold& A, const FBehaviorThreshold& B)
        {
            return A.Priority > B.Priority; // Higher priority first
        });

    // Check each threshold
    for (const FBehaviorThreshold& Threshold : SortedThresholds)
    {
        // Check relationship threshold
        if (RelationshipScore >= Threshold.RelationshipThreshold)
        {
            continue; // Relationship not bad enough for this behavior
        }

        // Check all required attributes
        bool bMeetsRequirements = true;
        for (const TPair<FName, float>& Requirement : Threshold.RequiredAttributes)
        {
            float AttributeValue = NPCAttributes->GetModifiedAttributeValue(Requirement.Key);
            if (AttributeValue < Requirement.Value)
            {
                bMeetsRequirements = false;
                break;
            }
        }

        if (bMeetsRequirements)
        {
            return Threshold.DecisionTag;
        }
    }

    // Default: ignore if no thresholds met
    return FVAITags::Behavior_Decision_Ignore;
}

FGameplayTag UNPCPersonalityProfile::SelectAttackStyle(UFVCharacterAttributeComponent* NPCAttributes) const
{
    if (!NPCAttributes)
    {
        return FVAITags::Attack_Style_CalculatedStrike;
    }

    // Sort by priority
    TArray<FAttackStyleRule> SortedStyles = AttackStyles;
    SortedStyles.Sort([](const FAttackStyleRule& A, const FAttackStyleRule& B)
        {
            return A.Priority > B.Priority;
        });

    // Find first matching style
    for (const FAttackStyleRule& Style : SortedStyles)
    {
        bool bMatches = true;

        // Check min attributes
        for (const TPair<FName, float>& MinReq : Style.MinAttributes)
        {
            if (NPCAttributes->GetModifiedAttributeValue(MinReq.Key) < MinReq.Value)
            {
                bMatches = false;
                break;
            }
        }

        if (!bMatches) continue;

        // Check max attributes
        for (const TPair<FName, float>& MaxReq : Style.MaxAttributes)
        {
            if (NPCAttributes->GetModifiedAttributeValue(MaxReq.Key) > MaxReq.Value)
            {
                bMatches = false;
                break;
            }
        }

        if (!bMatches) continue;

        // Check skills
        //for (const FName& SkillName : Style.RequiredSkills)
        //{
        //    if (NPCAttributes->GetSkillLevel(SkillName) < Style.SkillThreshold)
        //    {
        //        bMatches = false;
        //        break;
        //    }
        //}

        if (bMatches)
        {
            return Style.AttackStyleTag;
        }
    }

    // Default fallback
    return FVAITags::Attack_Style_CalculatedStrike;
}

void UNPCPersonalityProfile::ApplyConditionalModifiers(UFVCharacterAttributeComponent* NPCAttributes, TMap<FName, float>& OutModifiers) const
{
    if (!NPCAttributes)
    {
        return;
    }

    OutModifiers.Empty();

    for (const FConditionalModifier& Modifier : ConditionalModifiers)
    {
        float AttributeValue = NPCAttributes->GetModifiedAttributeValue(Modifier.ConditionAttribute);

        bool bConditionMet = Modifier.bGreaterThan
            ? (AttributeValue > Modifier.ConditionThreshold)
            : (AttributeValue < Modifier.ConditionThreshold);

        if (bConditionMet)
        {
            for (const TPair<FName, float>& ModPair : Modifier.AttributeModifiers)
            {
                float* ExistingMod = OutModifiers.Find(ModPair.Key);
                if (ExistingMod)
                {
                    *ExistingMod += ModPair.Value;
                }
                else
                {
                    OutModifiers.Add(ModPair.Key, ModPair.Value);
                }
            }
        }
    }
}