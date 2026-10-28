// Drops units of an inventory stack (payload instance id and quantity) in front of the character as a world item that falls with physics. Worn or held stacks, quest and key items stay.
class UFVDropItemAbility : UFVInteractAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_Drop;

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		Drop(FVItem::GetItemPayload(EventData));
		EndAbility();
	}

	private void Drop(UFVItemEventPayload Payload)
	{
		AActor Avatar = GetAvatarActorFromActorInfo();
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Avatar);
		FFVItemStack Stack;

		if (Payload == nullptr || !IsValid(Inventory) || !Inventory.GetStack(Payload.InstanceId, Stack) || !CanDrop(Avatar, Stack))
		{
			return;
		}

		FFVItemStack Dropped = Stack;
		Dropped.Quantity = Math::Clamp(Payload.Quantity, 1, Stack.Quantity);

		AActor WorldActor = FVItem::SpawnWorldItem(Dropped, FVItemWorld::GetDropTransform(Avatar));

		if (WorldActor == nullptr)
		{
			return;
		}

		if (!Inventory.RemoveStack(Stack.InstanceId, Dropped.Quantity))
		{
			WorldActor.DestroyActor();
			return;
		}

		FVItemWorld::EnablePhysics(WorldActor);

		FFVItemFragment_World ItemWorldFragment;

		if (FVItem::GetWorldFragment(Stack.Item, ItemWorldFragment))
		{
			FVItemWorld::PlaySound(ItemWorldFragment.DropSound, WorldActor.GetActorLocation());
		}
	}

	private bool CanDrop(AActor Avatar, FFVItemStack Stack)
	{
		if (Stack.Item == nullptr || Stack.Item.bQuestItem || Stack.Item.IsKeyItem())
		{
			return false;
		}

		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Avatar);

		return !IsValid(Equipment) || !Equipment.IsInstanceEquipped(Stack.InstanceId);
	}
}
