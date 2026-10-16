// Uses an item from the inventory (payload with an instance id or an item) or straight from the world: plays its use montage with a prop in the right hand, and the item takes effect on the Apply notify.
class UFVUseItemAbility : UFVItemMontageAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_Use;
	default ActivationOwnedTags.AddTag(GameplayTags::Status_Interacting);
	default ActivationOwnedTags.AddTag(GameplayTags::Status_HandsBusy);

	private UFVItemDefinition UsedItem;
	private FGuid UsedInstanceId;
	private AActor UsedWorldActor;
	private bool bFromWorld = false;
	private bool bWorldMeshHidden = false;
	private UStaticMeshComponent Prop;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		StartUse(nullptr);
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		StartUse(FVItem::GetItemPayload(EventData));
	}

	private void StartUse(UFVItemEventPayload Payload)
	{
		AActor Avatar = GetAvatarActorFromActorInfo();

		UsedItem = nullptr;
		UsedInstanceId = FGuid();
		UsedWorldActor = nullptr;
		bWorldMeshHidden = false;
		bFromWorld = Payload == nullptr || Payload.WorldActor != nullptr;

		if (bFromWorld)
		{
			UsedWorldActor = FVItemWorld::GetWorldTarget(this, Payload);
			UsedItem = FVItemWorld::GetWorldItem(UsedWorldActor);

			if (UsedItem == nullptr || !UFVPickupComponent::Get(UsedWorldActor).CanUseInPlace(Avatar))
			{
				EndAbility();
				return;
			}

			SetItemWarpTarget(UsedWorldActor.RootComponent);
		}
		else
		{
			UsedItem = Payload.Item;
			UsedInstanceId = Payload.InstanceId;

			if (!CanUseFromInventory(Avatar))
			{
				EndAbility();
				return;
			}
		}

		PlayItemMontage(FVItem::FindUseMontage(Avatar, UsedItem));
	}

	private bool CanUseFromInventory(AActor Avatar)
	{
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Avatar);

		if (!IsValid(Inventory))
		{
			return false;
		}

		if (!UsedInstanceId.IsValid())
		{
			return Inventory.CanUseItem(UsedItem);
		}

		FFVItemStack Stack;

		if (!Inventory.GetStack(UsedInstanceId, Stack))
		{
			return false;
		}

		UsedItem = Stack.Item;
		return Inventory.CanUseStack(UsedInstanceId);
	}

	void OnItemAttach() override
	{
		if (Prop != nullptr)
		{
			return;
		}

		Prop = FVItemAnim::AttachProp(GetAvatarActorFromActorInfo(), UsedItem, FVItem::GetHandSocket(true));
		SetWorldMeshVisible(false);
	}

	void OnItemApply() override
	{
		AActor Avatar = GetAvatarActorFromActorInfo();

		if (!(bFromWorld ? UseFromWorld(Avatar) : UseFromInventory(Avatar)))
		{
			return;
		}

		FFVItemFragment_World World;

		if (FVItem::GetWorldFragment(UsedItem, World))
		{
			FVItemWorld::PlaySound(World.UseSound, Avatar.GetActorLocation());
		}
	}

	void OnItemRelease() override
	{
		FVItemAnim::DetachProp(Prop);
		Prop = nullptr;
		SetWorldMeshVisible(true);
	}

	void OnItemAbilityEnded(bool bWasApplied) override
	{
		OnItemRelease();
		UsedItem = nullptr;
		UsedWorldActor = nullptr;
	}

	private bool UseFromInventory(AActor Avatar)
	{
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Avatar);

		if (!IsValid(Inventory))
		{
			return false;
		}

		return UsedInstanceId.IsValid() ? Inventory.UseStack(UsedInstanceId) : Inventory.UseItem(UsedItem);
	}

	// The pickup spends a charge or a unit like Use Stack; the actor handles a used up pickup.
	private bool UseFromWorld(AActor Avatar)
	{
		UFVPickupComponent Pickup = IsValid(UsedWorldActor) ? UFVPickupComponent::Get(UsedWorldActor) : nullptr;

		return IsValid(Pickup) && Pickup.UseInPlace(Avatar);
	}

	// The world item's mesh hides while its prop is in the hand, and comes back if units are left.
	private void SetWorldMeshVisible(bool bVisible)
	{
		AFVWorldItem WorldItem = bFromWorld ? Cast<AFVWorldItem>(UsedWorldActor) : nullptr;

		if (!IsValid(WorldItem) || bVisible != bWorldMeshHidden)
		{
			return;
		}

		bWorldMeshHidden = !bVisible;

		if (!bVisible || !WorldItem.Pickup.IsDepleted())
		{
			WorldItem.Mesh.SetVisibility(bVisible);
		}
	}
}
