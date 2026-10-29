// One filter tab of the storage list: items whose category matches Category (any when empty), optionally only wearables.
struct FFVInventoryFilter
{
    UPROPERTY(EditAnywhere, Category = "Filter")
    FText Label;

    UPROPERTY(EditAnywhere, Category = "Filter", meta = (Categories = "Item"))
    FGameplayTag Category;

    UPROPERTY(EditAnywhere, Category = "Filter")
    bool bWearableOnly = false;
}

// Inventory section listing the carried stacks. Selecting a row selects the stack in the shared model, so the
// Information section shows it and the Outfit section highlights where it can be worn.
UCLASS(Abstract)
class UFVInventoryStorageSection : UFVHubListSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Storage", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVInventoryFilter> Filters;

    UPROPERTY(EditDefaultsOnly, Category = "Storage")
    FText WornLabel;

    private UFVInventoryModel Model;

    UFUNCTION(BlueprintOverride)
    void OnSectionInitialized()
    {
        Model = FVInventoryUI::GetModel(this);
        if (Model != nullptr)
        {
            Model.OnChanged.AddUFunction(this, n"HandleModelChanged");
            Model.OnSelectionChanged.AddUFunction(this, n"HandleSelectionChanged");
        }
        Super::OnSectionInitialized();
    }

    TArray<FText> GetFilterLabels() override
    {
        TArray<FText> Labels;
        for (FFVInventoryFilter Filter : Filters)
        {
            Labels.Add(Filter.Label);
        }
        return Labels;
    }

    UFUNCTION(BlueprintOverride)
    int GetSlotCount()
    {
        int count = Model != nullptr ? Model.GetCellCapacity() : 0;
        return Model != nullptr ? Model.GetCellCapacity() : 0;
    }
    
    TArray<FFVListEntry> BuildEntries(int FilterIndex) override
    {
        TArray<FFVListEntry> Result;
        if (Model == nullptr)
        {
            return Result;
        }

        for (FFVItemStack Stack : Model.GetStacks())
        {
            if (Stack.Item == nullptr || !PassesFilter(Stack.Item, FilterIndex))
            {
                continue;
            }

            bool bWorn = Model.IsWorn(Stack.InstanceId);

            FFVListEntry Entry;
            Entry.Title = FVInventoryUI::GetItemName(Stack.Item);
            Entry.Subtitle = bWorn ? WornLabel : FVUIText::TagLeaf(Stack.Item.Category);
            Entry.Value = Stack.Quantity > 1 ? FText::FromString(f"x{Stack.Quantity}") : FText();
            Entry.Description = Stack.Item.Display.ShortDescription;
            Entry.Icon = Stack.Item.Display.Icon;
            Entry.bHighlighted = bWorn;
            Entry.Source = Stack.Item;
            Entry.Id = Stack.InstanceId;
            Result.Add(Entry);
        }
        return Result;
    }

    void OnEntrySelected(FFVListEntry Entry) override
    {
        if (Model != nullptr && Model.GetSelectedStackId() != Entry.Id)
        {
            Model.SelectStack(Entry.Id);
        }
    }

    private bool PassesFilter(UFVItemDefinition Item, int FilterIndex) const
    {
        if (!Filters.IsValidIndex(FilterIndex))
        {
            return true;
        }

        FFVInventoryFilter Filter = Filters[FilterIndex];
        if (Filter.bWearableOnly && !Item.IsEquippable())
        {
            return false;
        }
        return !Filter.Category.IsValid() || Item.Category.MatchesTag(Filter.Category);
    }

    UFUNCTION()
    private void HandleModelChanged()
    {
        RebuildList();
    }

    // Follows selections made elsewhere, e.g. clicking a worn item in the Outfit section.
    UFUNCTION()
    private void HandleSelectionChanged()
    {
        FFVListEntry Current;
        bool bHasCurrent = GetSelectedEntry(Current);
        FGuid Wanted = Model.GetSelectedStackId();
        if (!Wanted.IsValid() || (bHasCurrent && Current.Id == Wanted))
        {
            return;
        }

        TArray<FFVListEntry> Visible = BuildEntries(GetActiveFilter());
        for (int i = 0; i < Visible.Num(); i++)
        {
            if (Visible[i].Id == Wanted)
            {
                SelectEntry(i);
                return;
            }
        }
    }
}
