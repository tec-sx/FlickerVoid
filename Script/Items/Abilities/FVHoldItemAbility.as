// Takes an item into a hand slot: the payload's inventory stack, or the targeted world item after picking it up. The right hand if the item fits it (the left one when only that is free), else the left; holding a held stack again puts it away.
class UFVHoldItemAbility : UFVInteractAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_Hold;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		Hold(nullptr);
		EndAbility();
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		Hold(FVItem::GetItemPayload(EventData));
		EndAbility();
	}

	private void Hold(UFVItemEventPayload Payload)
	{
		AActor Avatar = GetAvatarActorFromActorInfo();
		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Avatar);
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Avatar);

		if (!IsValid(Equipment) || !IsValid(Inventory))
		{
			return;
		}

		if (Payload != nullptr && Payload.InstanceId.IsValid())
		{
			HoldStack(Equipment, Inventory, Payload.InstanceId);
			return;
		}

		AActor WorldActor = FVItemWorld::GetWorldTarget(this, Payload);
		UFVItemDefinition Item = FVItemWorld::GetWorldItem(WorldActor);

		if (Item == nullptr || !ChooseHand(Equipment, Item).IsValid())
		{
			return;
		}

		FGuid InstanceId = FVItemWorld::PickUpStack(Avatar, WorldActor);

		if (InstanceId.IsValid())
		{
			HoldStack(Equipment, Inventory, InstanceId);
		}
	}

	private void HoldStack(UFVEquipmentComponent Equipment, UFVInventoryComponent Inventory, FGuid InstanceId)
	{
		FGameplayTag HeldIn = FVItemAnim::GetHoldingHand(Equipment, InstanceId);

		if (HeldIn.IsValid())
		{
			Equipment.Unequip(HeldIn);
			return;
		}

		FFVItemStack Stack;

		if (!Inventory.GetStack(InstanceId, Stack))
		{
			return;
		}

		FGameplayTag Hand = ChooseHand(Equipment, Stack.Item);

		if (Hand.IsValid())
		{
			Equipment.EquipInstance(InstanceId, Hand);
		}
	}

	private FGameplayTag ChooseHand(UFVEquipmentComponent Equipment, UFVItemDefinition Item)
	{
		FGameplayTagContainer Slots = Item.GetEquipmentSlots();
		FGameplayTag Right = FVItemAnim::GetHandSlot(true);
		FGameplayTag Left = FVItemAnim::GetHandSlot(false);
		bool bFitsRight = Slots.HasTagExact(Right);
		bool bFitsLeft = Slots.HasTagExact(Left);

		if (bFitsRight && bFitsLeft && Equipment.GetEquipped(Right) != nullptr && Equipment.GetEquipped(Left) == nullptr)
		{
			return Left;
		}

		if (bFitsRight)
		{
			return Right;
		}

		if (bFitsLeft)
		{
			return Left;
		}

		return FGameplayTag();
	}
}
