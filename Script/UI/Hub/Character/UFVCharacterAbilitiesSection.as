// A tab of the Abilities section: the attributes whose category matches AttributeCategory, e.g. Attribute.Skill.
struct FFVAbilityTab
{
    UPROPERTY(EditAnywhere, Category = "Abilities")
    FText Label;

    UPROPERTY(EditAnywhere, Category = "Abilities", meta = (Categories = "Attribute"))
    FGameplayTag AttributeCategory;
}

// Character section for Skills, Perks and Kinks: one tab each, a list with the current values and a detail panel.
// Until the skill tree exists the data source is the player's FV Attribute Component: each tab lists the attributes
// under its category tag, and an attribute at its minimum shows as not acquired.
UCLASS(Abstract)
class UFVCharacterAbilitiesSection : UFVHubListSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVAbilityTab> Tabs;

    private UFVAttributeComponent Attributes;

    UFUNCTION(BlueprintOverride)
    void OnRefresh()
    {
        BindAttributes();
        Super::OnRefresh();
    }

    UFUNCTION(BlueprintOverride)
    void OnSectionHidden()
    {
        if (Attributes != nullptr)
        {
            Attributes.OnAttributeChanged.UnbindObject(this);
            Attributes = nullptr;
        }
    }

    TArray<FText> GetFilterLabels() override
    {
        TArray<FText> Labels;
        for (FFVAbilityTab Tab : Tabs)
        {
            Labels.Add(Tab.Label);
        }
        return Labels;
    }

    TArray<FFVListEntry> BuildEntries(int Filter) override
    {
        TArray<FFVListEntry> Result;
        if (Attributes == nullptr || !Tabs.IsValidIndex(Filter))
        {
            return Result;
        }

        FGameplayTag Category = Tabs[Filter].AttributeCategory;
        for (UFVAttributeDefinition Attribute : Attributes.GetKnownAttributes())
        {
            if (Attribute == nullptr || (Category.IsValid() && !Attribute.Category.MatchesTag(Category)))
            {
                continue;
            }

            float Value = Attributes.GetValue(Attribute);

            FFVListEntry Entry;
            Entry.Title = Attribute.Display.Name;
            Entry.Subtitle = Attribute.Display.ShortDescription;
            Entry.Value = FText::FromString(Attribute.bInteger ? f"{Math::RoundToInt(Value)}" : FVUIText::Decimal(Value));
            Entry.Description = Attribute.Display.LongDescription;
            Entry.Icon = Attribute.Display.Icon;
            Entry.Progress = FVUIText::Fraction(Value - Attribute.Min, Attribute.Max - Attribute.Min);
            Entry.bDimmed = Value <= Attribute.Min;
            Entry.Source = Attribute;
            Result.Add(Entry);
        }
        return Result;
    }

    private void BindAttributes()
    {
        UFVAttributeComponent Current = UFVAttributeComponent::Get(GetOwningPlayerPawn(), NAME_None);
        if (Current == Attributes)
        {
            return;
        }
        OnSectionHidden();
        Attributes = Current;
        if (Attributes != nullptr)
        {
            Attributes.OnAttributeChanged.AddUFunction(this, n"HandleAttributeChanged");
        }
    }

    UFUNCTION()
    private void HandleAttributeChanged(const UFVAttributeDefinition Attribute, float32 OldValue, float32 NewValue)
    {
        RebuildList();
    }
}
