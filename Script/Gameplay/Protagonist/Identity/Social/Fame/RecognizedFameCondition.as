/**
 * Fame as onlookers read it on the Instigator: a disguise hides most of it. For the real fame use the Scale condition
 * on the fame scale. At-least comparisons are scored.
 */
class URecognizedFameCondition : UFVCondition
{
    UPROPERTY(EditAnywhere, Category = "Condition")
    EFVCompare Compare = EFVCompare::GreaterOrEqual;

    UPROPERTY(EditAnywhere, Category = "Condition")
    int Value = 0;

    UFUNCTION(BlueprintOverride)
    bool Evaluate(FFVConditionContext Context) const
    {
        int Fame = FVSocial::GetRecognizedFame(Context.Instigator);
        return FVCondition::CompareValues(float(Fame), Compare, float(Value));
    }

    UFUNCTION(BlueprintOverride)
    bool Score(FFVConditionContext Context, FFVConditionScore&out OutScore) const
    {
        int Fame = FVSocial::GetRecognizedFame(Context.Instigator);
        return FVCondition::ScoreValue(float(Fame), Compare, float(Value), OutScore);
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString Comparison = FVCondition::DescribeCompare(Compare, float(Value)).ToString();
        return FText::FromString(f"Recognized fame {Comparison}");
    }
}
