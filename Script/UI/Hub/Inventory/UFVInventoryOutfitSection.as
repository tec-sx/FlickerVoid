// A tab of outfit slots, e.g. Clothes, Underwear, Accessories.
struct FFVOutfitSlotGroup
{
    UPROPERTY(EditAnywhere, Category = "Outfit")
    FText Label;

    UPROPERTY(EditAnywhere, Category = "Outfit", meta = (TitleProperty = "Label"))
    TArray<FFVOutfitSlot> Slots;
}

// Inventory section showing what is worn, slot by slot, in up to three groups. Clicking a slot wears the item selected
// in Storage there when it fits; otherwise it selects the slot so the Information section shows what is worn in it.
UCLASS(Abstract)
class UFVInventoryOutfitSection : UFVHubSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Outfit", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVOutfitSlotGroup> Groups;

    UPROPERTY(EditDefaultsOnly, Category = "Outfit")
    TSubclassOf<UFVOutfitSlotWidget> SlotWidgetClass;

    UPROPERTY(BindWidget)
    UPanelWidget SlotPanel;

    UPROPERTY(BindWidgetOptional)
    UFVTabBarWidget GroupTabs;

    private UFVInventoryModel Model;
    private TArray<UFVOutfitSlotWidget> SlotWidgets;
    private int ShownGroup = -1;

    UFUNCTION(BlueprintOverride)
    void OnSectionInitialized()
    {
        Model = FVInventoryUI::GetModel(this);
        if (Model != nullptr)
        {
            Model.OnChanged.AddUFunction(this, n"HandleModelChanged");
            Model.OnSelectionChanged.AddUFunction(this, n"HandleModelChanged");
        }

        if (GroupTabs != nullptr)
        {
            TArray<FText> Labels;
            for (FFVOutfitSlotGroup Group : Groups)
            {
                Labels.Add(Group.Label);
            }
            GroupTabs.OnTabIndexSelected.AddUFunction(this, n"HandleGroupSelected");
            GroupTabs.SetTabs(Labels);
            GroupTabs.SelectTab(0);
        }
        else
        {
            ShowGroup(-1);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnRefresh()
    {
        RefreshSlots();
    }

    UFUNCTION(BlueprintOverride)
    UWidget GetFocusTarget()
    {
        return SlotWidgets.Num() > 0 ? SlotWidgets[0] : nullptr;
    }

    // Group -1 lists the slots of every group.
    private void ShowGroup(int GroupIndex)
    {
        ShownGroup = GroupIndex;
        SlotPanel.ClearChildren();
        SlotWidgets.Empty();

        if (!SlotWidgetClass.IsValid())
        {
            Print(f"{GetName()}: Slot Widget Class is not set.");
            return;
        }

        for (int i = 0; i < Groups.Num(); i++)
        {
            if (GroupIndex >= 0 && i != GroupIndex)
            {
                continue;
            }

            for (FFVOutfitSlot SlotInfo : Groups[i].Slots)
            {
                UFVOutfitSlotWidget SlotWidget = WidgetBlueprint::CreateWidget(SlotWidgetClass, GetOwningPlayer());
                SlotWidget.Setup(SlotInfo);
                SlotWidget.OnClicked.AddUFunction(this, n"HandleSlotClicked");
                SlotPanel.AddChild(SlotWidget);
                SlotWidgets.Add(SlotWidget);
            }
        }

        RefreshSlots();
    }

    private void RefreshSlots()
    {
        if (Model == nullptr)
        {
            return;
        }

        FGuid Selected = Model.GetSelectedStackId();
        bool bSelectionCarried = Selected.IsValid() && !Model.IsWorn(Selected);

        for (UFVOutfitSlotWidget SlotWidget : SlotWidgets)
        {
            FGameplayTag SlotTag = SlotWidget.GetSlot();
            bool bCanTake = bSelectionCarried && Model.CanEquip(Selected, SlotTag);
            SlotWidget.Refresh(Model, Model.GetSelectedSlot() == SlotTag, bCanTake);
        }
    }

    UFUNCTION()
    private void HandleGroupSelected(int Index)
    {
        ShowGroup(Index);
    }

    UFUNCTION()
    private void HandleModelChanged()
    {
        RefreshSlots();
    }

    UFUNCTION()
    private void HandleSlotClicked(UFVOutfitSlotWidget SlotWidget)
    {
        if (Model == nullptr)
        {
            return;
        }

        FGameplayTag SlotTag = SlotWidget.GetSlot();
        FGuid Selected = Model.GetSelectedStackId();
        if (Selected.IsValid() && Model.GetWornSlot(Selected) != SlotTag && Model.CanEquip(Selected, SlotTag))
        {
            Model.Equip(Selected, SlotTag);
            Model.SelectSlot(SlotTag);
            return;
        }

        Model.SelectSlot(SlotTag);
    }
}
