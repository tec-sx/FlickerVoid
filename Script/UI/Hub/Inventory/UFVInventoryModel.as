event void FFVOnInventoryModelChanged();

// The inventory page's view of the player's FV Inventory and FV Equipment components: contents, capacity, the
// selected stack or outfit slot, and the actions (add, use, equip, unequip, drop). Sections read it and listen to
// OnChanged; nothing here keeps its own copy of the inventory.
class UFVInventoryModel : UObject
{
    UPROPERTY()
    FFVOnInventoryModelChanged OnChanged;

    UPROPERTY()
    FFVOnInventoryModelChanged OnSelectionChanged;

    private UFVInventoryComponent Inventory;
    private UFVEquipmentComponent Equipment;
    private FGuid SelectedStack;
    private FGameplayTag SelectedSlot;

    void Bind(AActor Owner)
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
            Print(f"Inventory menu: {Owner.GetName()} has no FV Inventory Component.");
        }

        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.AddUFunction(this, n"HandleEquipmentChanged");
            Equipment.OnEquipmentReset.AddUFunction(this, n"HandleChanged");
        }
    }

    void Unbind()
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

    bool HasInventory() const
    {
        return Inventory != nullptr;
    }

    bool HasEquipment() const
    {
        return Equipment != nullptr;
    }

    // ---- Contents and capacity ----

    TArray<FFVItemStack> GetStacks() const
    {
        return Inventory != nullptr ? Inventory.GetItems() : TArray<FFVItemStack>();
    }

    bool GetStack(FGuid InstanceId, FFVItemStack& OutStack) const
    {
        return Inventory != nullptr && Inventory.GetStack(InstanceId, OutStack);
    }

    float GetTotalWeight() const
    {
        return Inventory != nullptr ? Inventory.GetTotalWeight() : 0.0;
    }

    float GetWeightLimit() const
    {
        return Inventory != nullptr ? Inventory.GetWeightLimit() : 0.0;
    }

    int GetUsedCells() const
    {
        return Inventory != nullptr ? Inventory.GetUsedCells() : 0;
    }

    int GetCellCapacity() const
    {
        return Inventory != nullptr ? Inventory.GetCellCapacity() : 0;
    }

    // ---- Outfit ----

    bool IsWorn(FGuid InstanceId) const
    {
        return Equipment != nullptr && Equipment.IsInstanceEquipped(InstanceId);
    }

    // The slot a carried stack is worn in, or an invalid tag.
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

    UFVItemDefinition GetWornItem(FGameplayTag Slot) const
    {
        return Equipment != nullptr ? Equipment.GetEquipped(Slot) : nullptr;
    }

    FGuid GetWornInstance(FGameplayTag Slot) const
    {
        return Equipment != nullptr ? Equipment.GetEquippedInstance(Slot) : FGuid();
    }

    // The worn item covering this slot, e.g. a dress blocking Lower Body.
    UFVItemDefinition GetBlockingItem(FGameplayTag Slot) const
    {
        return Equipment != nullptr ? Equipment.GetBlockingItem(Slot) : nullptr;
    }

    TArray<FFVEquippedItem> GetAllWorn() const
    {
        return Equipment != nullptr ? Equipment.GetAllEquipped() : TArray<FFVEquippedItem>();
    }

    // ---- Selection ----

    void SelectStack(FGuid InstanceId)
    {
        SelectedStack = InstanceId;
        SelectedSlot = GetWornSlot(InstanceId);
        OnSelectionChanged.Broadcast();
    }

    // Selects an outfit slot and whatever is worn there.
    void SelectSlot(FGameplayTag Slot)
    {
        SelectedSlot = Slot;
        SelectedStack = GetWornInstance(Slot);
        OnSelectionChanged.Broadcast();
    }

    void ClearSelection()
    {
        SelectedStack = FGuid();
        SelectedSlot = FGameplayTag();
        OnSelectionChanged.Broadcast();
    }

    FGuid GetSelectedStackId() const
    {
        return SelectedStack;
    }

    FGameplayTag GetSelectedSlot() const
    {
        return SelectedSlot;
    }

    bool GetSelectedStack(FFVItemStack& OutStack) const
    {
        return GetStack(SelectedStack, OutStack);
    }

    // ---- Actions ----

    // Adds units to the inventory, limited by weight and space; returns how many fit.
    int AddItem(UFVItemDefinition Item, int Quantity = 1)
    {
        return Inventory != nullptr && Item != nullptr ? Inventory.AddItem(Item, Quantity) : 0;
    }

    bool CanUse(FGuid InstanceId) const
    {
        FFVItemStack Stack;
        return GetStack(InstanceId, Stack) && Stack.Item.IsUsable() && Inventory.CanUseItem(Stack.Item);
    }

    // Applies the item's Usable effects; a consumed unit comes from an unworn plain stack of the item first.
    bool Use(FGuid InstanceId)
    {
        FFVItemStack Stack;
        if (!CanUse(InstanceId) || !GetStack(InstanceId, Stack))
        {
            return false;
        }
        return Inventory.UseItem(Stack.Item);
    }

    // Slot None picks the item's first free slot.
    bool CanEquip(FGuid InstanceId, FGameplayTag Slot) const
    {
        return Equipment != nullptr && Equipment.CanEquipInstance(InstanceId, Slot);
    }

    bool Equip(FGuid InstanceId, FGameplayTag Slot)
    {
        return Equipment != nullptr && Equipment.EquipInstance(InstanceId, Slot);
    }

    bool CanUnequip(FGameplayTag Slot) const
    {
        return Equipment != nullptr && Slot.IsValid() && Equipment.CanUnequip(Slot);
    }

    bool Unequip(FGameplayTag Slot)
    {
        return CanUnequip(Slot) && Equipment.Unequip(Slot);
    }

    bool CanToggleWorn(FGuid InstanceId) const
    {
        FGameplayTag WornSlot = GetWornSlot(InstanceId);
        return WornSlot.IsValid() ? CanUnequip(WornSlot) : CanEquip(InstanceId, FGameplayTag());
    }

    // Takes the stack off if worn, otherwise wears it in its first free slot.
    bool ToggleWorn(FGuid InstanceId)
    {
        FGameplayTag WornSlot = GetWornSlot(InstanceId);
        return WornSlot.IsValid() ? Unequip(WornSlot) : Equip(InstanceId, FGameplayTag());
    }

    // Same rules as the drop ability: not quest, key or worn items.
    bool CanDrop(FGuid InstanceId, int Quantity) const
    {
        FFVItemStack Stack;
        
        if (Inventory == nullptr || Quantity < 1 || !Inventory.GetStack(InstanceId, Stack) || Quantity > Stack.Quantity)
        {
            return false;
        }
        
        if (Stack.Item == nullptr || Stack.Item.bQuestItem || Stack.Item.IsKeyItem())
        {
            return false;
        }
        
        return Equipment == nullptr || !Equipment.IsInstanceEquipped(InstanceId);
    }

    // Sends Event.Item.Drop; the player's drop ability spawns the world item.
    bool Drop(FGuid InstanceId, int Quantity)
    {
        FFVItemStack Stack;
        
        if (!CanDrop(InstanceId, Quantity) || !Inventory.GetStack(InstanceId, Stack))
        {
            return false;
        }
        
        return FVItem::SendItemEvent(Inventory.GetOwner(), GameplayTags::Event_Item_Drop, Stack.Item, InstanceId, nullptr, nullptr, Quantity);
    }

    UFUNCTION()
    private void HandleStackChanged(FGuid InstanceId)
    {
        HandleChanged();
    }

    UFUNCTION()
    private void HandleEquipmentChanged(FGameplayTag SlotTag, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
    {
        HandleChanged();
    }

    UFUNCTION()
    private void HandleChanged()
    {
        FFVItemStack Selected;
        if (SelectedStack.IsValid() && !GetStack(SelectedStack, Selected))
        {
            SelectedStack = SelectedSlot.IsValid() ? GetWornInstance(SelectedSlot) : FGuid();
            OnSelectionChanged.Broadcast();
        }
        OnChanged.Broadcast();
    }
}
