// HUD notifications, newest first, at most three on screen: items gained or lost, quests started, completed or
// failed, objectives done, titles earned or lost, and new journal knowledge.
UCLASS(Abstract)
class UFVNotificationFeedWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Notifications")
    TSubclassOf<UFVListEntryWidget> EntryClass;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications", meta = (ClampMin = 1, ClampMax = 3))
    int MaxVisible = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications", meta = (ClampMin = 0.5, Units = "s"))
    float Duration = 4.0;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications", meta = (ToolTip = "Root of the knowledge Id tags, e.g. Knowledge; learning one shows its name with the Knowledge label."))
    FGameplayTag KnowledgeFactRoot;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText ItemAddedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText ItemRemovedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText QuestStartedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText QuestUpdatedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText QuestCompletedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText QuestFailedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText TitleEarnedLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText TitleLostLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Notifications|Labels")
    FText KnowledgeLabel;

    UPROPERTY(BindWidget)
    UPanelWidget NotificationList;

    private TArray<FFVListEntry> Shown;
    private TArray<float> ExpireTimes;
    private TArray<UFVListEntryWidget> Rows;
    private TMap<UFVQuestDefinition, EFVQuestState> QuestStates;
    private UFVInventoryComponent Inventory;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        APlayerController Player = GetOwningPlayer();
        if (Player != nullptr)
        {
            Player.OnPossessedPawnChanged.AddUFunction(this, n"HandlePawnChanged");
            BindInventory(Player.GetControlledPawn());
        }

        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests != nullptr)
        {
            for (EFVQuestState State : GetTrackedStates())
            {
                for (UFVQuestDefinition Quest : Quests.GetQuests(State))
                {
                    QuestStates.Add(Quest, State);
                }
            }
            Quests.OnQuestsChanged.AddUFunction(this, n"HandleQuestsChanged");
        }

        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        if (Social != nullptr)
        {
            Social.OnTitleEarned.AddUFunction(this, n"HandleTitleEarned");
            Social.OnTitleLost.AddUFunction(this, n"HandleTitleLost");
        }

        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.AddUFunction(this, n"HandleFactsChanged");
        }

        Render();
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        APlayerController Player = GetOwningPlayer();
        if (Player != nullptr)
        {
            Player.OnPossessedPawnChanged.UnbindObject(this);
        }
        BindInventory(nullptr);

        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        if (Quests != nullptr)
        {
            Quests.OnQuestsChanged.UnbindObject(this);
        }
        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        if (Social != nullptr)
        {
            Social.OnTitleEarned.UnbindObject(this);
            Social.OnTitleLost.UnbindObject(this);
        }
        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.UnbindObject(this);
        }
    }

    UFUNCTION(BlueprintOverride)
    void Tick(FGeometry MyGeometry, float InDeltaTime)
    {
        float Now = System::GetGameTimeInSeconds();
        bool bExpired = false;
        for (int i = ExpireTimes.Num() - 1; i >= 0; i--)
        {
            if (ExpireTimes[i] <= Now)
            {
                ExpireTimes.RemoveAt(i);
                Shown.RemoveAt(i);
                bExpired = true;
            }
        }
        if (bExpired)
        {
            Render();
        }
    }

    // Adds a notification on top; the oldest drops off past MaxVisible.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void Notify(FText Title, FText Subtitle, TSoftObjectPtr<UTexture2D> Icon)
    {
        FFVListEntry Entry;
        Entry.Title = Title;
        Entry.Subtitle = Subtitle;
        Entry.Icon = Icon;

        Shown.Insert(Entry, 0);
        ExpireTimes.Insert(System::GetGameTimeInSeconds() + Duration, 0);

        int Limit = Math::Clamp(MaxVisible, 1, 3);
        while (Shown.Num() > Limit)
        {
            Shown.RemoveAt(Shown.Num() - 1);
            ExpireTimes.RemoveAt(ExpireTimes.Num() - 1);
        }
        Render();
    }

    private void Render()
    {
        if (!EntryClass.IsValid())
        {
            return;
        }

        while (Rows.Num() < Shown.Num())
        {
            UFVListEntryWidget Row = WidgetBlueprint::CreateWidget(EntryClass, GetOwningPlayer());
            NotificationList.AddChild(Row);
            Rows.Add(Row);
        }

        for (int i = 0; i < Rows.Num(); i++)
        {
            bool bUsed = i < Shown.Num();
            Rows[i].SetVisibility(bUsed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
            if (bUsed)
            {
                Rows[i].SetEntry(Shown[i], i);
            }
        }
    }

    private TArray<EFVQuestState> GetTrackedStates() const
    {
        TArray<EFVQuestState> States;
        States.Add(EFVQuestState::Active);
        States.Add(EFVQuestState::Completed);
        States.Add(EFVQuestState::Failed);
        return States;
    }

    private void BindInventory(APawn Pawn)
    {
        if (Inventory != nullptr)
        {
            Inventory.OnItemChanged.UnbindObject(this);
        }
        Inventory = Pawn != nullptr ? UFVInventoryComponent::Get(Pawn) : nullptr;
        if (Inventory != nullptr)
        {
            Inventory.OnItemChanged.AddUFunction(this, n"HandleItemChanged");
        }
    }

    UFUNCTION()
    private void HandlePawnChanged(APawn OldPawn, APawn NewPawn)
    {
        BindInventory(NewPawn);
    }

    UFUNCTION()
    private void HandleItemChanged(UFVItemDefinition Item, int OldQuantity, int NewQuantity)
    {
        if (Item == nullptr || OldQuantity == NewQuantity)
        {
            return;
        }

        int Delta = NewQuantity - OldQuantity;
        FString Amount = Math::Abs(Delta) > 1 ? f" x{Math::Abs(Delta)}" : "";
        FText Label = Delta > 0 ? ItemAddedLabel : ItemRemovedLabel;
        Notify(FVInventoryUI::GetItemName(Item), FText::FromString(f"{Label}{Amount}"), Item.Display.Icon);
    }

    UFUNCTION()
    private void HandleQuestsChanged()
    {
        UFVQuestSubsystem Quests = UFVQuestSubsystem::Get();
        UFVQuestDefinition Quest = Quests != nullptr ? Quests.GetLastChangedQuest() : nullptr;
        if (Quest == nullptr)
        {
            return;
        }

        EFVQuestState State = Quests.GetState(Quest);
        EFVQuestState Previous = EFVQuestState::NotStarted;
        QuestStates.Find(Quest, Previous);
        QuestStates.Add(Quest, State);

        FText Label;
        if (State == Previous)
        {
            Label = State == EFVQuestState::Active ? QuestUpdatedLabel : FText();
        }
        else if (State == EFVQuestState::Active)
        {
            Label = QuestStartedLabel;
        }
        else if (State == EFVQuestState::Completed)
        {
            Label = QuestCompletedLabel;
        }
        else if (State == EFVQuestState::Failed)
        {
            Label = QuestFailedLabel;
        }

        if (!Label.IsEmpty())
        {
            Notify(Quest.Display.Name, Label, Quest.Display.Icon);
        }
    }

    UFUNCTION()
    private void HandleTitleEarned(const UFVTitleDefinition Title)
    {
        if (Title != nullptr)
        {
            Notify(Title.Display.Name, TitleEarnedLabel, Title.Display.Icon);
        }
    }

    UFUNCTION()
    private void HandleTitleLost(const UFVTitleDefinition Title)
    {
        if (Title != nullptr)
        {
            Notify(Title.Display.Name, TitleLostLabel, Title.Display.Icon);
        }
    }

    UFUNCTION()
    private void HandleFactsChanged()
    {
        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts == nullptr || !KnowledgeFactRoot.IsValid())
        {
            return;
        }

        FGameplayTag Changed = Facts.GetLastChangedTag();
        if (!Changed.MatchesTag(KnowledgeFactRoot) || Facts.GetFact(Changed) <= 0)
        {
            return;
        }

        for (UFVKnowledgeDefinition Knowledge : FVKnowledge::GetKnownKnowledge(TArray<EFVKnowledgeKind>()))
        {
            if (Knowledge.Id == Changed)
            {
                Notify(Knowledge.Display.Name, KnowledgeLabel, Knowledge.Display.Icon);
                return;
            }
        }
    }
}
