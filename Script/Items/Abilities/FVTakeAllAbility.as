// Takes everything from a loot container (AFVLootContainer): a reach montage (Animation.Item.Pickup.Mid) warped to its Interaction Point, then the container's Take All on the Apply notify. A locked container needs a fitting key.
class UFVTakeAllAbility : UFVItemMontageAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_TakeAll;
	default ActivationOwnedTags.AddTag(GameplayTags::Status_Interacting);
	default ActivationOwnedTags.AddTag(GameplayTags::Status_HandsBusy);

	private AFVLootContainer Container;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		StartTakeAll(IsValid(Interactable) ? Interactable.GetOwner() : nullptr);
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		StartTakeAll(FVItemWorld::GetWorldTarget(this, FVItem::GetItemPayload(EventData)));
	}

	private void StartTakeAll(AActor Target)
	{
		Container = Cast<AFVLootContainer>(Target);
		AActor Avatar = GetAvatarActorFromActorInfo();

		if (!IsValid(Container) || !CanOpen(Avatar))
		{
			EndAbility();
			return;
		}

		SetItemWarpTarget(Container.InteractionPoint);
		PlayItemMontage(FVItem::FindMontage(Avatar, GameplayTags::Animation_Item_Pickup_Mid));
	}

	private bool CanOpen(AActor Avatar)
	{
		UFVLockComponent Lock = UFVLockComponent::Get(Container);

		return !IsValid(Lock) || !Lock.IsLocked() || FVItemKeys::HasKeyFor(Avatar, Lock);
	}

	void OnItemApply() override
	{
		if (IsValid(Container))
		{
			Container.TakeAll(GetAvatarActorFromActorInfo());
		}
	}

	void OnItemAbilityEnded(bool bWasApplied) override
	{
		Container = nullptr;
	}
}
