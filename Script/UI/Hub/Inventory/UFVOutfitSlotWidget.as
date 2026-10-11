// An outfit slot as the Outfit section lists it.
struct FFVOutfitSlot
{
    UPROPERTY(EditAnywhere, Category = "Outfit", meta = (Categories = "Equipment.Slot"))
    FGameplayTag Slot;

    UPROPERTY(EditAnywhere, Category = "Outfit")
    FText Label;
}

event void FFVOnOutfitSlotClicked(UFVOutfitSlotWidget SlotWidget);

// One outfit slot: its label and what is worn there, or what covers it. Style the states in OnSlotStateChanged.
UCLASS(Abstract)
class UFVOutfitSlotWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UButton SlotButton;

    UPROPERTY(BindWidget)
    UTextBlock SlotLabelText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ItemNameText;

    UPROPERTY(BindWidgetOptional)
    UImage ItemIcon;

    UPROPERTY()
    FFVOnOutfitSlotClicked OnClicked;

    private FFVOutfitSlot SlotInfo;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        SlotButton.OnClicked.AddUFunction(this, n"HandleClicked");
    }

    void Setup(FFVOutfitSlot InSlot)
    {
        SlotInfo = InSlot;
        SlotLabelText.SetText(SlotInfo.Label.IsEmpty() ? FVUIText::TagLeaf(SlotInfo.Slot) : SlotInfo.Label);
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    FGameplayTag GetSlot() const
    {
        return SlotInfo.Slot;
    }

    // bCanTakeSelection: the item selected in Storage can be worn here.
    void Refresh(UFVInventoryModel Model, bool bSelected, bool bCanTakeSelection)
    {
        UFVItemDefinition Worn = Model.GetWornItem(SlotInfo.Slot);
        UFVItemDefinition Blocking = Worn == nullptr ? Model.GetBlockingItem(SlotInfo.Slot) : nullptr;
        UFVItemDefinition Shown = Worn != nullptr ? Worn : Blocking;

        if (ItemNameText != nullptr)
        {
            ItemNameText.SetText(FVInventoryUI::GetItemName(Shown));
        }

        if (ItemIcon != nullptr)
        {
            bool bHasIcon = Shown != nullptr && !Shown.Display.Icon.IsNull();
            ItemIcon.SetVisibility(bHasIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
            if (bHasIcon)
            {
                ItemIcon.SetBrushFromSoftTexture(Shown.Display.Icon, false);
            }
        }

        OnSlotStateChanged(Worn != nullptr, Blocking != nullptr, bSelected, bCanTakeSelection);
    }

    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnSlotStateChanged(bool bWorn, bool bBlocked, bool bSelected, bool bCanTakeSelection) {}

    UFUNCTION()
    private void HandleClicked()
    {
        OnClicked.Broadcast(this);
    }
}
