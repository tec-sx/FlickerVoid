/**
 * The player's relationship with a character. Empty Character means the character of the context Target, so one
 * dialogue line works for every NPC. At-least comparisons are scored.
 */
class URelationshipCondition : UFVCondition
{
    UPROPERTY(EditAnywhere, Category = "Condition")
    UFVCharacterDefinition Character;

    UPROPERTY(EditAnywhere, Category = "Condition")
    EFVCompare Compare = EFVCompare::GreaterOrEqual;

    UPROPERTY(EditAnywhere, Category = "Condition")
    int Value = 0;

    UFUNCTION(BlueprintOverride)
    bool Evaluate(FFVConditionContext Context) const
    {
        UFVScaleDefinition Relationship = FindRelationship(Context);
        if (Relationship == nullptr)
        {
            return false;
        }

        int Current = FVScale::GetValue(Relationship);
        return FVCondition::CompareValues(float(Current), Compare, float(Value));
    }

    UFUNCTION(BlueprintOverride)
    bool Score(FFVConditionContext Context, FFVConditionScore&out OutScore) const
    {
        UFVScaleDefinition Relationship = FindRelationship(Context);
        if (Relationship == nullptr)
        {
            return false;
        }

        int Current = FVScale::GetValue(Relationship);
        return FVCondition::ScoreValue(float(Current), Compare, float(Value), OutScore);
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString CharacterName = Character != nullptr ? Character.Display.Name.ToString() : "Target";
        FString Comparison = FVCondition::DescribeCompare(Compare, float(Value)).ToString();
        return FText::FromString(f"Relationship with {CharacterName} {Comparison}");
    }

    private UFVScaleDefinition FindRelationship(FFVConditionContext Context) const
    {
        UFVCharacterDefinition Resolved = Character != nullptr ? Character : UFVIdentityComponent::GetActorDefinition(Context.Target);
        return FVSocial::GetRelationshipScale(Resolved);
    }
}
