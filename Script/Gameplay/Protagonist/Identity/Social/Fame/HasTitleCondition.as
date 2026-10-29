/** The player has earned the title, whether or not it is the current one. */
class UHasTitleCondition : UFVCondition
{
    UPROPERTY(EditAnywhere, Category = "Condition")
    UFVTitleDefinition Title;

    UFUNCTION(BlueprintOverride)
    bool Evaluate(FFVConditionContext Context) const
    {
        return FVSocial::HasTitle(Title);
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString TitleName = Title != nullptr ? Title.Display.Name.ToString() : "<none>";
        return FText::FromString(f"Has title {TitleName}");
    }
}
