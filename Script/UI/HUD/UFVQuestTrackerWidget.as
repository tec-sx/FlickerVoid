// HUD quest tracker: up to three active quests, the most recently changed first, each with its current objective.
UCLASS(Abstract)
class UFVQuestTrackerWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Quest Tracker")
    TSubclassOf<UFVListEntryWidget> EntryClass;

    UPROPERTY(EditDefaultsOnly, Category = "Quest Tracker", meta = (ClampMin = 1, ClampMax = 3))
    int MaxQuests = 3;

    UPROPERTY(BindWidget)
    UPanelWidget QuestList;

    private TArray<UFVQuestDefinition> Order;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests != nullptr)
        {
            Quests.OnQuestsChanged.AddUFunction(this, n"HandleQuestsChanged");
        }
        Rebuild();
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

    UFUNCTION()
    private void HandleQuestsChanged()
    {
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        UFVQuestDefinition Changed = Quests != nullptr ? Quests.GetLastChangedQuest() : nullptr;
        if (Changed != nullptr)
        {
            Order.Remove(Changed);
            Order.Insert(Changed, 0);
        }
        Rebuild();
    }

    private void Rebuild()
    {
        QuestList.ClearChildren();
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests == nullptr || !EntryClass.IsValid())
        {
            SetVisibility(ESlateVisibility::Collapsed);
            return;
        }

        TArray<UFVQuestDefinition> Active = Quests.GetQuests(EFVQuestState::Active);
        TArray<UFVQuestDefinition> Shown;
        for (UFVQuestDefinition Quest : Order)
        {
            if (Active.Contains(Quest))
            {
                Shown.Add(Quest);
            }
        }
        for (UFVQuestDefinition Quest : Active)
        {
            if (!Shown.Contains(Quest))
            {
                Shown.Add(Quest);
            }
        }

        int Count = Math::Min(Shown.Num(), Math::Clamp(MaxQuests, 1, 3));
        for (int i = 0; i < Count; i++)
        {
            FFVListEntry Entry;
            Entry.Title = Shown[i].Display.Name;
            Entry.Subtitle = FVJournalUI::GetCurrentObjective(Shown[i]);
            Entry.Icon = Shown[i].Display.Icon;
            Entry.Source = Shown[i];

            UFVListEntryWidget Widget = WidgetBlueprint::CreateWidget(EntryClass, GetOwningPlayer());
            Widget.SetEntry(Entry, i);
            QuestList.AddChild(Widget);
        }

        SetVisibility(Count > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}
