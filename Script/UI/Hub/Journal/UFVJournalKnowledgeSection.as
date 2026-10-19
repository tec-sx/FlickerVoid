// A tab of a knowledge section: the known knowledge of these kinds.
struct FFVKnowledgeFilter
{
    UPROPERTY(EditAnywhere, Category = "Knowledge")
    FText Label;

    UPROPERTY(EditAnywhere, Category = "Knowledge")
    TArray<EFVKnowledgeKind> Kinds;
}

// Journal section listing what the player knows, read from the facts through FVStorySystem's knowledge library.
// Notes and Memories are two Blueprints of this class with different filters (clues and topics, thoughts).
UCLASS(Abstract)
class UFVJournalKnowledgeSection : UFVHubListSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Knowledge", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVKnowledgeFilter> Filters;

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
        for (FFVKnowledgeFilter Filter : Filters)
        {
            Labels.Add(Filter.Label);
        }
        return Labels;
    }

    TArray<FFVListEntry> BuildEntries(int Filter) override
    {
        TArray<EFVKnowledgeKind> Kinds;
        if (Filters.IsValidIndex(Filter))
        {
            Kinds = Filters[Filter].Kinds;
        }

        TArray<FFVListEntry> Result;
        for (UFVKnowledgeDefinition Knowledge : FVKnowledge::GetKnownKnowledge(Kinds))
        {
            FFVListEntry Entry;
            Entry.Title = Knowledge.Display.Name;
            Entry.Subtitle = Knowledge.Display.ShortDescription;
            Entry.Description = Knowledge.Display.LongDescription.IsEmpty() ? Knowledge.Display.ShortDescription : Knowledge.Display.LongDescription;
            Entry.Icon = Knowledge.Display.Icon;
            Entry.Source = Knowledge;
            Result.Add(Entry);
        }
        return Result;
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
