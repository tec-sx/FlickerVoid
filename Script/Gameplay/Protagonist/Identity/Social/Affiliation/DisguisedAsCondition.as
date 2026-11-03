/** The subject passes as a member of the faction: wears its disguise and isn't too notorious for it to work. */
class UDisguisedAsCondition : UFVCondition
{
    UPROPERTY(EditAnywhere, Category = "Condition")
    UFVFactionDefinition Faction;

    UPROPERTY(EditAnywhere, Category = "Condition")
    EFVContextActor Subject = EFVContextActor::Instigator;

    UFUNCTION(BlueprintOverride)
    bool Evaluate(FFVConditionContext Context) const
    {
        AActor SubjectActor = Subject == EFVContextActor::Target ? Context.Target : Context.Instigator;
        return FVSocial::IsDisguisedAs(SubjectActor, Faction);
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString FactionName = Faction != nullptr ? Faction.Display.Name.ToString() : "<none>";
        return FText::FromString(f"Disguised as {FactionName}");
    }
}
