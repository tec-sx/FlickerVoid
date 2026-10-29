// Character section listing where the player stands: factions (standing, tier, attitude, disguise) on one tab and
// relationships with characters met on the other. All values are facts read through FVSocialSystem.
UCLASS(Abstract)
class UFVCharacterSocialSection : UFVHubListSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Social")
    FText FactionsLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Social")
    FText RelationshipsLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Social")
    FText DisguisedAsLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Social", meta = (ToolTip = "Labels for Hostile, Unfriendly, Neutral, Friendly and Allied, in that order."))
    TArray<FText> AttitudeLabels;

    UFUNCTION(BlueprintOverride)
    void OnSectionInitialized()
    {
        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.AddUFunction(this, n"HandleFactsChanged");
        }
        Super::OnSectionInitialized();
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.UnbindObject(this);
        }
    }

    TArray<FText> GetFilterLabels() override
    {
        TArray<FText> Labels;
        Labels.Add(FactionsLabel);
        Labels.Add(RelationshipsLabel);
        return Labels;
    }

    TArray<FFVListEntry> BuildEntries(int Filter) override
    {
        return Filter == 0 ? BuildFactions() : BuildRelationships();
    }

    private TArray<FFVListEntry> BuildFactions()
    {
        TArray<FFVListEntry> Result;
        APawn Player = GetOwningPlayerPawn();

        for (UFVFactionDefinition Faction : FVSocial::GetFactions())
        {
            int Standing = FVSocial::GetStanding(Faction);
            bool bDisguised = FVSocial::IsDisguisedAs(Player, Faction);

            FString Subtitle = GetTierLabel(Faction, Standing).ToString();
            FString Attitude = GetAttitudeLabel(FVSocial::GetAttitude(Faction, Player)).ToString();
            if (!Attitude.IsEmpty())
            {
                Subtitle = Subtitle.IsEmpty() ? Attitude : f"{Subtitle} - {Attitude}";
            }
            if (bDisguised)
            {
                Subtitle = f"{Subtitle} ({DisguisedAsLabel})";
            }

            FFVListEntry Entry;
            Entry.Title = Faction.Display.Name;
            Entry.Subtitle = FText::FromString(Subtitle);
            Entry.Value = FText::FromString(f"{Standing}");
            Entry.Description = Faction.Display.LongDescription;
            Entry.Icon = Faction.Display.Icon;
            Entry.Progress = FVUIText::Fraction(float(Standing - Faction.MinStanding), float(Faction.MaxStanding - Faction.MinStanding));
            Entry.bHighlighted = bDisguised;
            Entry.Source = Faction;
            Result.Add(Entry);
        }
        return Result;
    }

    private TArray<FFVListEntry> BuildRelationships()
    {
        TArray<FFVListEntry> Result;
        for (UFVCharacterDefinition Character : FVSocial::GetKnownCharacters())
        {
            FFVListEntry Entry;
            Entry.Title = Character.Display.Name;
            Entry.Subtitle = Character.Display.ShortDescription;
            Entry.Value = FText::FromString(f"{FVSocial::GetRelationship(Character)}");
            Entry.Description = Character.Display.LongDescription;
            Entry.Icon = Character.Display.Icon;
            Entry.Source = Character;
            Result.Add(Entry);
        }
        return Result;
    }

    // Tiers are sorted ascending; the last one whose minimum is reached applies.
    private FText GetTierLabel(UFVFactionDefinition Faction, int Standing) const
    {
        FText Label;
        for (FFVReputationTier Tier : Faction.Tiers)
        {
            if (Standing >= Tier.MinStanding)
            {
                Label = Tier.Label;
            }
        }
        return Label;
    }

    private FText GetAttitudeLabel(EFVAttitude Attitude) const
    {
        int Index = int(Attitude);
        return AttitudeLabels.IsValidIndex(Index) ? AttitudeLabels[Index] : FText();
    }

    UFUNCTION()
    private void HandleFactsChanged()
    {
        if (IsVisible())
        {
            RebuildList();
        }
    }
}
