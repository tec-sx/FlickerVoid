// Config lives in the action's HandlerConfig slot — extend UFVInteractionHandlerConfig.
class UFVPickupHandlerConfig : UFVInteractionHandlerConfig
{
    // Inventory component method tag to call on success
    // (wires into FVItems inventory without a hard module dependency)
    UPROPERTY(Category = "Pickup")
    FGameplayTag InventoryActionTag;

    UPROPERTY(Category = "Pickup")
    FText PickupMessage = FText::FromString("Item added to inventory.");
}

// Immediate (synchronous) pickup handler.
// Fires a gameplay event on the instigator so FVItems can listen and add the item.
class UFVPickupHandler : UFVInteractionActionHandler
{
    UFVPickupHandlerConfig PickupConfig;

    UFUNCTION(BlueprintOverride)
    void OnInitialize(UFVInteractionHandlerConfig Config)
    {
        PickupConfig = Cast<UFVPickupHandlerConfig>(Config);
        if (PickupConfig == nullptr)
        {
            Warning(GetName() + ": no UFVPickupHandlerConfig assigned, using defaults.");
            PickupConfig = NewObject(this, UFVPickupHandlerConfig);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnExecute(FFVInteractionContext Context)
    {
        // Fire a gameplay event — FVItems listens for this tag and adds the item.
        // This keeps FVGameplay free of any FVItems dependency.
        if (PickupConfig.InventoryActionTag.IsValid())
        {
            FGameplayEventData EventData;
            EventData.Instigator = Context.Instigator;
            EventData.Target     = Context.TargetActor;
            EventData.EventTag   = PickupConfig.InventoryActionTag;

            AbilitySystem::SendGameplayEventToActor(
                Context.Instigator,
                PickupConfig.InventoryActionTag,
                EventData);
        }

        // Hide the world actor — the item is now in inventory
        if (IsValid(Context.TargetActor))
        {
            Context.TargetActor.SetActorHiddenInGame(true);
            Context.TargetActor.SetActorEnableCollision(false);
        }

        CompleteExecution(true);
    }

    UFUNCTION(BlueprintOverride)
    void OnCancel(FFVInteractionContext Context)
    {
        // Sync handler — cancel is a no-op (already completed or not started)
    }
}
