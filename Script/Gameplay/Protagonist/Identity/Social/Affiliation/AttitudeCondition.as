/**
 * How an observer's faction treats the other context actor, at least. In dialogue the observer is the Target (the
 * NPC); in an NPC's StateTree it is the Instigator (the NPC itself).
 */
class UAttitudeCondition : UFVCondition
{
    UPROPERTY(EditAnywhere, Category = "Condition")
    EFVContextActor Observer = EFVContextActor::Target;

    UPROPERTY(EditAnywhere, Category = "Condition", meta = (ToolTip = "Overrides the observer's own faction."))
    UFVFactionDefinition Faction;

    UPROPERTY(EditAnywhere, Category = "Condition")
    EFVAttitude AtLeast = EFVAttitude::Neutral;

    UFUNCTION(BlueprintOverride)
    bool Evaluate(FFVConditionContext Context) const
    {
        bool bObserverIsTarget = Observer == EFVContextActor::Target;
        AActor ObserverActor = bObserverIsTarget ? Context.Target : Context.Instigator;
        AActor Subject = bObserverIsTarget ? Context.Instigator : Context.Target;

        UFVFactionDefinition ObserverFaction = Faction != nullptr ? Faction : FVSocial::GetActorFaction(ObserverActor);
        if (ObserverFaction == nullptr)
        {
            return false;
        }

        EFVAttitude Attitude = FVSocial::GetAttitude(ObserverFaction, Subject);
        return int(Attitude) >= int(AtLeast);
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString FactionName = Faction != nullptr ? Faction.Display.Name.ToString() : "Observer's faction";
        return FText::FromString(f"{FactionName} at least {AtLeast}");
    }
}
