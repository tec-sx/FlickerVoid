// Inventory section describing the selected stack (name, icon, description, category, quantity, weight, size,
// where it is worn) with its three actions, Use, Wear/Take off and Drop, plus the carried weight and space.
UCLASS(Abstract)
class UFVInventoryInfoSection : UFVHubSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText WearLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText TakeOffLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText WornInLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText QuestItemLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText CannotUseText;

    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText CannotWearText;

    UPROPERTY(EditDefaultsOnly, Category = "Information")
    FText CannotDropText;

    UPROPERTY(BindWidgetOptional)
    UWidget ItemPanel;

    UPROPERTY(BindWidgetOptional)
    UWidget NoSelectionPanel;

    UPROPERTY(BindWidgetOptional)
    UImage ItemIcon;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ItemNameText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock CategoryText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DescriptionText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock QuantityText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ItemWeightText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock SizeText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock StatusText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock FeedbackText;

    UPROPERTY(BindWidgetOptional)
    UButton UseButton;

    UPROPERTY(BindWidgetOptional)
    UButton WearButton;

    UPROPERTY(BindWidgetOptional)
    UTextBlock WearButtonText;

    UPROPERTY(BindWidgetOptional)
    UButton DropButton;

    UPROPERTY(BindWidgetOptional)
    USpinBox DropQuantity;

    UPROPERTY(BindWidgetOptional)
    UTextBlock TotalWeightText;

    UPROPERTY(BindWidgetOptional)
    UProgressBar WeightBar;

    UPROPERTY(BindWidgetOptional)
    UTextBlock SpaceText;

    UPROPERTY(BindWidgetOptional)
    UProgressBar SpaceBar;

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

        if (UseButton != nullptr)
        {
            UseButton.OnClicked.AddUFunction(this, n"HandleUseClicked");
        }
        if (WearButton != nullptr)
        {
            WearButton.OnClicked.AddUFunction(this, n"HandleWearClicked");
        }
        if (DropButton != nullptr)
        {
            DropButton.OnClicked.AddUFunction(this, n"HandleDropClicked");
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnRefresh()
    {
        ShowFeedback(FText());
        RefreshAll();
    }

    UFUNCTION(BlueprintOverride)
    UWidget GetFocusTarget()
    {
        return UseButton;
    }

    private void RefreshAll()
    {
        RefreshCapacity();
        RefreshItem();
    }

    private void RefreshCapacity()
    {
        if (Model == nullptr)
        {
            return;
        }

        float Weight = Model.GetTotalWeight();
        float Limit = Model.GetWeightLimit();
        if (TotalWeightText != nullptr)
        {
            TotalWeightText.SetText(FText::FromString(f"{FVUIText::Decimal(Weight)} / {FVUIText::Decimal(Limit)} kg"));
        }
        if (WeightBar != nullptr)
        {
            WeightBar.SetPercent(float32(FVUIText::Fraction(Weight, Limit)));
        }

        int Used = Model.GetUsedCells();
        int Capacity = Model.GetCellCapacity();
        if (SpaceText != nullptr)
        {
            SpaceText.SetText(FVUIText::Ratio(f"{Used}", f"{Capacity}"));
        }
        if (SpaceBar != nullptr)
        {
            SpaceBar.SetPercent(float32(FVUIText::Fraction(float(Used), float(Capacity))));
        }
    }

    private void RefreshItem()
    {
        FFVItemStack Stack;
        bool bHasItem = Model != nullptr && Model.GetSelectedStack(Stack) && Stack.Item != nullptr;

        if (ItemPanel != nullptr)
        {
            ItemPanel.SetVisibility(bHasItem ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
        }
        if (NoSelectionPanel != nullptr)
        {
            NoSelectionPanel.SetVisibility(bHasItem ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
        }

        if (!bHasItem)
        {
            SetButtonState(UseButton, false, false);
            SetButtonState(WearButton, false, false);
            SetButtonState(DropButton, false, false);
            return;
        }

        UFVItemDefinition Item = Stack.Item;
        FGameplayTag WornSlot = Model.GetWornSlot(Stack.InstanceId);

        SetText(ItemNameText, FVInventoryUI::GetItemName(Item));
        SetText(CategoryText, FVUIText::TagLeaf(Item.Category));
        SetText(DescriptionText, Item.Display.LongDescription.IsEmpty() ? Item.Display.ShortDescription : Item.Display.LongDescription);
        SetText(QuantityText, Item.MaxQuantity == 1 && Stack.Quantity == 1 ? FText() : FText::FromString(f"x{Stack.Quantity}"));
        SetText(ItemWeightText, Stack.Quantity > 1
            ? FText::FromString(f"{FVUIText::Decimal(Item.Weight)} kg each, {FVUIText::Decimal(Item.Weight * Stack.Quantity)} kg")
            : FVUIText::Weight(Item.Weight));
        SetText(SizeText, FText::FromString(f"{Math::Max(1, Item.GridSize.X)} x {Math::Max(1, Item.GridSize.Y)}"));
        SetText(StatusText, GetStatus(Item, WornSlot));

        if (ItemIcon != nullptr)
        {
            ItemIcon.SetVisibility(Item.Display.Icon.IsNull() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
            if (!Item.Display.Icon.IsNull())
            {
                ItemIcon.SetBrushFromSoftTexture(Item.Display.Icon, false);
            }
        }

        if (WearButtonText != nullptr)
        {
            WearButtonText.SetText(WornSlot.IsValid() ? TakeOffLabel : WearLabel);
        }

        SetButtonState(UseButton, Item.IsUsable(), Model.CanUse(Stack.InstanceId));
        SetButtonState(WearButton, Item.IsEquippable(), Model.CanToggleWorn(Stack.InstanceId));
        SetButtonState(DropButton, !Item.bQuestItem, Model.CanDrop(Stack.InstanceId, GetDropQuantity(Stack)));

        if (DropQuantity != nullptr)
        {
            DropQuantity.SetMinValue(1.0);
            DropQuantity.SetMaxValue(float32(Math::Max(1, Stack.Quantity)));
            DropQuantity.SetValue(float32(Math::Clamp(GetDropQuantity(Stack), 1, Stack.Quantity)));
            DropQuantity.SetVisibility(Stack.Quantity > 1 && !Item.bQuestItem ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }
    }

    private FText GetStatus(UFVItemDefinition Item, FGameplayTag WornSlot) const
    {
        if (WornSlot.IsValid())
        {
            return FText::FromString(f"{WornInLabel} {FVUIText::TagLeaf(WornSlot)}");
        }
        if (Item.bQuestItem)
        {
            return QuestItemLabel;
        }
        return FText();
    }

    private int GetDropQuantity(FFVItemStack Stack) const
    {
        if (DropQuantity == nullptr || DropQuantity.GetVisibility() == ESlateVisibility::Collapsed)
        {
            return 1;
        }
        return Math::Clamp(Math::RoundToInt(DropQuantity.GetValue()), 1, Math::Max(1, Stack.Quantity));
    }

    private void SetButtonState(UButton Button, bool bShown, bool bEnabled)
    {
        if (Button != nullptr)
        {
            Button.SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
            Button.SetIsEnabled(bEnabled);
        }
    }

    private void SetText(UTextBlock Block, FText Text)
    {
        if (Block != nullptr)
        {
            Block.SetText(Text);
            Block.SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
        }
    }

    private void ShowFeedback(FText Text)
    {
        SetText(FeedbackText, Text);
    }

    UFUNCTION()
    private void HandleModelChanged()
    {
        RefreshAll();
    }

    UFUNCTION()
    private void HandleSelectionChanged()
    {
        ShowFeedback(FText());
        RefreshItem();
    }

    UFUNCTION()
    private void HandleUseClicked()
    {
        if (!Model.Use(Model.GetSelectedStackId()))
        {
            ShowFeedback(CannotUseText);
        }
    }

    UFUNCTION()
    private void HandleWearClicked()
    {
        if (!Model.ToggleWorn(Model.GetSelectedStackId()))
        {
            ShowFeedback(CannotWearText);
        }
    }

    UFUNCTION()
    private void HandleDropClicked()
    {
        FFVItemStack Stack;
        if (!Model.GetSelectedStack(Stack) || Model.Drop(Stack.InstanceId, GetDropQuantity(Stack)) == nullptr)
        {
            ShowFeedback(CannotDropText);
        }
    }
}
