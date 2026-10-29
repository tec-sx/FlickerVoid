/** Makes a title the player's current one, earning it first if needed. It stays current until another title takes the lead. */
class UPromoteTitleEffect : UFVEffect
{
    UPROPERTY(EditAnywhere, Category = "Effect")
    UFVTitleDefinition Title;

    UFUNCTION(BlueprintOverride)
    void Apply(FFVConditionContext Context) const
    {
        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        if (Social != nullptr)
        {
            Social.PromoteTitle(Title);
        }
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString TitleName = Title != nullptr ? Title.Display.Name.ToString() : "<none>";
        return FText::FromString(f"Promote title {TitleName}");
    }
}
