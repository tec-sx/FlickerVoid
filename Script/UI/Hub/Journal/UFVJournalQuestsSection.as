// Journal section listing quests from the quest subsystem: Active, Completed and Failed tabs, and for the selected
// quest its description and visible objectives (done, optional). Quest states are facts, so this follows saves too.
UCLASS(Abstract)
class UFVJournalQuestsSection : UFVHubListSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Quests")
    FText ActiveLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Quests")
    FText CompletedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Quests")
    FText FailedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Quests")
    FText OptionalLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Quests")
    TSubclassOf<UFVListEntryWidget> ObjectiveEntryClass;

    UPROPERTY(BindWidgetOptional)
    UPanelWidget ObjectiveList;

    UFUNCTION(BlueprintOverride)
    void OnSectionInitialized()
    {
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests != nullptr)
        {
            Quests.OnQuestsChanged.AddUFunction(this, n"HandleQuestsChanged");
        }
        Super::OnSectionInitialized();
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests != nullptr)
        {
            Quests.OnQuestsChanged.UnbindObject(this);
        }
    }

    TArray<FText> GetFilterLabels() override
    {
        TArray<FText> Labels;
        Labels.Add(ActiveLabel);
        Labels.Add(CompletedLabel);
        Labels.Add(FailedLabel);
        return Labels;
    }

    TArray<FFVListEntry> BuildEntries(int Filter) override
    {
        TArray<FFVListEntry> Result;
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests == nullptr)
        {
            return Result;
        }

        EFVQuestState State = Filter == 1 ? EFVQuestState::Completed : (Filter == 2 ? EFVQuestState::Failed : EFVQuestState::Active);
        for (UFVQuestDefinition Quest : Quests.GetQuests(State))
        {
            FFVListEntry Entry;
            Entry.Title = Quest.Display.Name;
            Entry.Subtitle = State == EFVQuestState::Active ? FVJournalUI::GetCurrentObjective(Quest) : Quest.Display.ShortDescription;
            Entry.Description = Quest.Display.LongDescription.IsEmpty() ? Quest.Display.ShortDescription : Quest.Display.LongDescription;
            Entry.Icon = Quest.Display.Icon;
            Entry.bHighlighted = State == EFVQuestState::Completed;
            Entry.bDimmed = State == EFVQuestState::Failed;
            Entry.Source = Quest;
            Result.Add(Entry);
        }
        return Result;
    }

    void OnEntrySelected(FFVListEntry Entry) override
    {
        ShowObjectives(Cast<UFVQuestDefinition>(Entry.Source));
    }

    void OnSelectionCleared() override
    {
        ShowObjectives(nullptr);
    }

    private void ShowObjectives(UFVQuestDefinition Quest)
    {
        if (ObjectiveList == nullptr)
        {
            return;
        }

        ObjectiveList.ClearChildren();
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quest == nullptr || Quests == nullptr || !ObjectiveEntryClass.IsValid())
        {
            return;
        }

        int Index = 0;
        for (FFVQuestObjective Objective : Quest.Objectives)
        {
            if (!Quests.IsObjectiveVisible(Objective))
            {
                continue;
            }

            FFVListEntry Row;
            Row.Title = Objective.Description;
            Row.Subtitle = Objective.bOptional ? OptionalLabel : FText();
            Row.bHighlighted = Quests.IsObjectiveComplete(Objective);

            UFVListEntryWidget Widget = WidgetBlueprint::CreateWidget(ObjectiveEntryClass, GetOwningPlayer());
            Widget.SetEntry(Row, Index);
            ObjectiveList.AddChild(Widget);
            Index++;
        }
    }

    UFUNCTION()
    private void HandleQuestsChanged()
    {
        RebuildList();
    }
}

namespace FVJournalUI
{
    // First visible objective that isn't complete, required ones first.
    FText GetCurrentObjective(UFVQuestDefinition Quest)
    {
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quest == nullptr || Quests == nullptr)
        {
            return FText();
        }

        FText Optional;
        for (FFVQuestObjective Objective : Quest.Objectives)
        {
            if (!Quests.IsObjectiveVisible(Objective) || Quests.IsObjectiveComplete(Objective))
            {
                continue;
            }
            if (!Objective.bOptional)
            {
                return Objective.Description;
            }
            if (Optional.IsEmpty())
            {
                Optional = Objective.Description;
            }
        }
        return Optional;
    }
}
