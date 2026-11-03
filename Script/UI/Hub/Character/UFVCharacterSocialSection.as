// Character section listing where the player stands: factions (standing, band, attitude, disguise) on one tab and
// relationships with characters met on the other. Standing and relationships are scales.
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
            int Standing = FVScale::GetValue(Faction.Standing);
            bool bDisguised = FVSocial::IsDisguisedAs(Player, Faction);
            EFVAttitude FactionAttitude = FVSocial::GetAttitude(Faction, Player);

            FString Subtitle = GetBandLabel(Faction.Standing).ToString();
            FString Attitude = GetAttitudeLabel(FactionAttitude).ToString();
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
            Entry.Progress = FVUIText::ScaleFraction(Faction.Standing);
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
            UFVScaleDefinition Relationship = FVSocial::GetRelationshipScale(Character);
            int Value = FVScale::GetValue(Relationship);

            FFVListEntry Entry;
            Entry.Title = Character.Display.Name;
            Entry.Subtitle = Character.Display.ShortDescription;
            Entry.Value = FText::FromString(f"{Value}");
            Entry.Progress = FVUIText::ScaleFraction(Relationship);
            Entry.Description = Character.Display.LongDescription;
            Entry.Icon = Character.Display.Icon;
            Entry.Source = Character;
            Result.Add(Entry);
        }
        return Result;
    }

    private FText GetBandLabel(UFVScaleDefinition Scale) const
    {
        FFVScaleBand Band;
        return FVScale::GetBand(Scale, Band) ? Band.Label : FText();
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
