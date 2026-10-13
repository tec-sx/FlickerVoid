// What the inventory page knows about one stack, for an information panel: Get Item Info fills it.
struct FFVInventoryItemInfo
{
    // The current stack: refreshed from the inventory when it is carried.
    UPROPERTY()
    FFVItemStack Stack;

    UPROPERTY()
    FText Name;

    // The long description, or the short one when there is none.
    UPROPERTY()
    FText Description;

    // Last part of the item's category tag.
    UPROPERTY()
    FText Category;

    UPROPERTY()
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY()
    float UnitWeight = 0.0;

    UPROPERTY()
    float TotalWeight = 0.0;

    // Footprint in cells, width x height.
    UPROPERTY()
    FIntPoint Size;

    // The slot the stack is worn in; empty when it isn't worn.
    UPROPERTY()
    FGameplayTag WornSlot;

    UPROPERTY()
    bool bQuestItem = false;

    UPROPERTY()
    bool bUsable = false;

    UPROPERTY()
    bool bWearable = false;

    // The actions below are false for stacks the player doesn't carry (loot, a trader's stock).
    UPROPERTY()
    bool bCanUse = false;

    UPROPERTY()
    bool bCanWear = false;

    UPROPERTY()
    bool bCanTakeOff = false;

    // At least one unit can be dropped.
    UPROPERTY()
    bool bCanDrop = false;
}

// Hub page for the inventory, laid out freely in its Widget Blueprint: place an FV Item Grid, FV Equipment Slots and an
// information panel and wire them in the graph. This class reads the player's inventory and equipment and does the
// actions. While the page is shown, On Inventory Changed fires whenever the contents, the outfit or the capacity change.
UCLASS(Abstract)
class UFVInventoryPageWidget : UFVHubMenuPageWidget
{
    private UFVInventoryComponent Inventory;
    private UFVEquipmentComponent Equipment;

    void ShowPage() override
    {
        Bind(GetOwningPlayerPawn());
        Super::ShowPage();
    }

    void HidePage() override
    {
        Super::HidePage();
        Unbind();
    }

    // The contents, the outfit or the capacity changed while the page is shown: refill the grid and panels here.
    UFUNCTION(BlueprintEvent, Category = "FV|Inventory")
    void OnInventoryChanged() {}

    // ---- Contents ----

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    TArray<FFVItemStack> GetInventoryItems() const
    {
        return Inventory != nullptr ? Inventory.GetItems() : TArray<FFVItemStack>();
    }

    // Carried stacks whose category matches Category (any when empty), optionally only wearable ones.
    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    TArray<FFVItemStack> GetFilteredItems(FGameplayTag Category, bool bWearableOnly) const
    {
        TArray<FFVItemStack> Result;
        for (FFVItemStack Stack : GetInventoryItems())
        {
            if (Stack.Item == nullptr || (bWearableOnly && !Stack.Item.IsEquippable()))
            {
                continue;
            }
            if (!Category.IsValid() || Stack.Item.Category.MatchesTag(Category))
            {
                Result.Add(Stack);
            }
        }
        return Result;
    }

    // Everything an information panel shows about a stack, e.g. the item grid's hovered item.
    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    FFVInventoryItemInfo GetItemInfo(FFVItemStack Item) const
    {
        FFVItemStack Carried;
        bool bCarried = Inventory != nullptr && Item.InstanceId.IsValid() && Inventory.GetStack(Item.InstanceId, Carried);

        FFVInventoryItemInfo Info;
        Info.Stack = bCarried ? Carried : Item;

        UFVItemDefinition Definition = Info.Stack.Item;
        if (Definition == nullptr)
        {
            return Info;
        }

        Info.Name = FVInventoryUI::GetItemName(Definition);
        Info.Description = Definition.Display.LongDescription.IsEmpty() ? Definition.Display.ShortDescription : Definition.Display.LongDescription;
        Info.Category = FVUIText::TagLeaf(Definition.Category);
        Info.Icon = Definition.Display.Icon;
        Info.UnitWeight = Definition.Weight;
        Info.TotalWeight = Definition.Weight * Info.Stack.Quantity;
        Info.Size = FIntPoint(Math::Max(1, Definition.GridSize.X), Math::Max(1, Definition.GridSize.Y));
        Info.bQuestItem = Definition.bQuestItem;
        Info.bUsable = Definition.IsUsable();
        Info.bWearable = Definition.IsEquippable();

        if (bCarried)
        {
            Info.WornSlot = GetWornSlot(Info.Stack.InstanceId);
            Info.bCanUse = CanUse(Info.Stack.InstanceId);
            Info.bCanWear = !Info.WornSlot.IsValid() && CanWear(Info.Stack.InstanceId, FGameplayTag());
            Info.bCanTakeOff = Info.WornSlot.IsValid() && CanTakeOff(Info.WornSlot);
            Info.bCanDrop = CanDrop(Info.Stack.InstanceId, 1);
        }
        return Info;
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    UFVInventoryComponent GetInventory() const
    {
        return Inventory;
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    UFVEquipmentComponent GetEquipment() const
    {
        return Equipment;
    }

    // ---- Capacity ----

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    float GetTotalWeight() const
    {
        return Inventory != nullptr ? Inventory.GetTotalWeight() : 0.0;
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    float GetWeightLimit() const
    {
        return Inventory != nullptr ? Inventory.GetWeightLimit() : 0.0;
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    int GetUsedCells() const
    {
        return Inventory != nullptr ? Inventory.GetUsedCells() : 0;
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    int GetCellCapacity() const
    {
        return Inventory != nullptr ? Inventory.GetCellCapacity() : 0;
    }

    // ---- Outfit ----

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    bool IsWorn(FGuid InstanceId) const
    {
        return Equipment != nullptr && Equipment.IsInstanceEquipped(InstanceId);
    }

    // The slot a carried stack is worn in; empty when it isn't worn.
    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    FGameplayTag GetWornSlot(FGuid InstanceId) const
    {
        if (Equipment != nullptr && InstanceId.IsValid())
        {
            for (FFVEquippedItem Entry : Equipment.GetAllEquipped())
            {
                if (Entry.InstanceId == InstanceId)
                {
                    return Entry.Slot;
                }
            }
        }
        return FGameplayTag();
    }

    // ---- Actions ----

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    bool CanUse(FGuid InstanceId) const
    {
        FFVItemStack Stack;
        return Inventory != nullptr && Inventory.GetStack(InstanceId, Stack) && Stack.Item.IsUsable() && Inventory.CanUseItem(Stack.Item);
    }

    // Applies the item's Usable effects; a consumed unit comes from an unworn plain stack of the item first.
    UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
    bool UseItem(FGuid InstanceId)
    {
        FFVItemStack Stack;
        if (!CanUse(InstanceId) || !Inventory.GetStack(InstanceId, Stack))
        {
            return false;
        }
        return Inventory.UseItem(Stack.Item);
    }

    // Slot Tag None checks the item's first free slot.
    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    bool CanWear(FGuid InstanceId, FGameplayTag SlotTag) const
    {
        return Equipment != nullptr && Equipment.CanEquipInstance(InstanceId, SlotTag);
    }

    // Wears the stack in Slot Tag, or in its first free slot when Slot Tag is None; whatever conflicts comes off.
    UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
    bool Wear(FGuid InstanceId, FGameplayTag SlotTag)
    {
        return Equipment != nullptr && Equipment.EquipInstance(InstanceId, SlotTag);
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    bool CanTakeOff(FGameplayTag SlotTag) const
    {
        return Equipment != nullptr && SlotTag.IsValid() && Equipment.CanUnequip(SlotTag);
    }

    UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
    bool TakeOff(FGameplayTag SlotTag)
    {
        return CanTakeOff(SlotTag) && Equipment.Unequip(SlotTag);
    }

    // Takes the stack off if it is worn, otherwise wears it in its first free slot.
    UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
    bool ToggleWorn(FGuid InstanceId)
    {
        FGameplayTag WornSlot = GetWornSlot(InstanceId);
        return WornSlot.IsValid() ? TakeOff(WornSlot) : Wear(InstanceId, FGameplayTag());
    }

    UFUNCTION(BlueprintPure, Category = "FV|Inventory")
    bool CanDrop(FGuid InstanceId, int Quantity) const
    {
        return Inventory != nullptr && Inventory.CanDropStack(InstanceId, Quantity);
    }

    // Spawns the units as a pickup in front of the player; a worn stack dropped whole comes off.
    UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
    bool DropItem(FGuid InstanceId, int Quantity)
    {
        return CanDrop(InstanceId, Quantity) && Inventory.DropStack(InstanceId, Quantity) != nullptr;
    }

    private void Bind(AActor Owner)
    {
        Unbind();
        if (Owner == nullptr)
        {
            return;
        }

        Inventory = UFVInventoryComponent::Get(Owner);
        Equipment = UFVEquipmentComponent::Get(Owner);

        if (Inventory != nullptr)
        {
            Inventory.OnStackChanged.AddUFunction(this, n"HandleStackChanged");
            Inventory.OnInventoryReset.AddUFunction(this, n"HandleChanged");
            Inventory.OnCapacityChanged.AddUFunction(this, n"HandleChanged");
        }
        else
        {
            Print(f"Inventory page: {Owner.GetName()} has no FV Inventory Component.");
        }

        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.AddUFunction(this, n"HandleEquipmentChanged");
            Equipment.OnEquipmentReset.AddUFunction(this, n"HandleChanged");
        }
    }

    private void Unbind()
    {
        if (Inventory != nullptr)
        {
            Inventory.OnStackChanged.UnbindObject(this);
            Inventory.OnInventoryReset.UnbindObject(this);
            Inventory.OnCapacityChanged.UnbindObject(this);
        }
        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.UnbindObject(this);
            Equipment.OnEquipmentReset.UnbindObject(this);
        }
        Inventory = nullptr;
        Equipment = nullptr;
    }

    UFUNCTION()
    private void HandleStackChanged(FGuid InstanceId)
    {
        OnInventoryChanged();
    }

    UFUNCTION()
    private void HandleEquipmentChanged(FGameplayTag ChangedSlot, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
    {
        OnInventoryChanged();
    }

    UFUNCTION()
    private void HandleChanged()
    {
        OnInventoryChanged();
    }
}

namespace FVInventoryUI
{
    FText GetItemName(UFVItemDefinition Item)
    {
        if (Item == nullptr)
        {
            return FText();
        }
        return Item.Display.Name.IsEmpty() ? FText::FromString(Item.GetName().ToString()) : Item.Display.Name;
    }
}
