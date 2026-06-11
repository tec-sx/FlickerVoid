// Sync pickup task.
// Fires a gameplay event so FVItems can add the item without a hard module
// dependency, then hides the world actor. Completes in the same frame.
//
// Configure in the State Tree editor:
//   InventoryActionTag — event tag FVItems listens for (e.g. Inventory.Event.AddItem)

class UFVPickupTask : UFVInteractionStateTreeTaskBase
{
    UPROPERTY(Category = "Pickup")
    FGameplayTag InventoryActionTag;

    UPROPERTY(Category = "Pickup")
    FText PickupMessage = FText::FromString("Item added to inventory.");

    UFUNCTION(BlueprintOverride)
    EStateTreeStateChangeType ReceiveEnterState(AActor OwnerActor,
        EStateTreeActionType ActionType, FStateTreeTransitionResult Transition)
    {
        AActor Instigator = GetInstigator(OwnerActor);

        if (InventoryActionTag.IsValid() && IsValid(Instigator))
        {
            FGameplayEventData EventData;
            EventData.Instigator = Instigator;
            EventData.Target     = OwnerActor;
            EventData.EventTag   = InventoryActionTag;

            AbilitySystem::SendGameplayEventToActor(Instigator, InventoryActionTag, EventData);
        }

        if (IsValid(OwnerActor))
        {
            OwnerActor.SetActorHiddenInGame(true);
            OwnerActor.SetActorEnableCollision(false);
        }

        CompleteTask(OwnerActor, true);
        return EStateTreeStateChangeType::Changed;
    }
}
