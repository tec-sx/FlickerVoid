// Wears an item: the payload's inventory stack, or the targeted world item after picking it up.
class UFVWearItemAbility : UFVInteractAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_Wear;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		Wear(nullptr);
		EndAbility();
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		Wear(FVItem::GetItemPayload(EventData));
		EndAbility();
	}

	private void Wear(UFVItemEventPayload Payload)
	{
		AActor Avatar = GetAvatarActorFromActorInfo();
		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Avatar);

		if (!IsValid(Equipment))
		{
			return;
		}

		if (Payload != nullptr && Payload.InstanceId.IsValid())
		{
			Equipment.EquipInstance(Payload.InstanceId, FGameplayTag());
			return;
		}

		AActor WorldActor = FVItemWorld::GetWorldTarget(this, Payload);
		UFVItemDefinition Item = FVItemWorld::GetWorldItem(WorldActor);

		if (Item == nullptr || !Item.IsEquippable())
		{
			return;
		}

		FGuid InstanceId = FVItemWorld::PickUpStack(Avatar, WorldActor);

		if (InstanceId.IsValid())
		{
			Equipment.EquipInstance(InstanceId, FGameplayTag());
		}
	}
}
