// Picks up the targeted world item with the character's montage for the pickup height, taking it on the Apply notify; collects money at once.
class UFVPickupAbility : UFVItemMontageAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ToolTip = "Used when the character has no montage for the pickup style."))
	UAnimMontage PickupMontage;

	default TriggerEventTag = GameplayTags::Event_Item_Pickup;
	default ActivationOwnedTags.AddTag(GameplayTags::Status_Interacting);
	default ActivationOwnedTags.AddTag(GameplayTags::Status_HandsBusy);

	// Captured on activation so a focus change during the montage can't redirect the pickup.
	private AActor PickupTarget;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		StartPickup(IsValid(Interactable) ? Interactable.GetOwner() : nullptr);
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		StartPickup(FVItemWorld::GetWorldTarget(this, FVItem::GetItemPayload(EventData)));
	}

	private void StartPickup(AActor Target)
	{
		PickupTarget = Target;
		AActor Avatar = GetAvatarActorFromActorInfo();

		if (!IsValid(PickupTarget))
		{
			EndAbility();
			return;
		}

		AFVMoneyPickup Money = Cast<AFVMoneyPickup>(PickupTarget);

		if (Money != nullptr)
		{
			Money.Collect(Avatar);
			EndAbility();
			return;
		}

		UFVPickupComponent Pickup = UFVPickupComponent::Get(PickupTarget);

		if (!IsValid(Pickup) || !Pickup.CanBePickedUpBy(Avatar))
		{
			EndAbility();
			return;
		}

		UAnimMontage Montage = FVItem::FindMontage(Avatar, FVItem::GetPickupStyle(Avatar, PickupTarget.GetActorLocation()));

		if (Montage == nullptr)
		{
			Montage = PickupMontage;
		}

		SetItemWarpTarget(PickupTarget.RootComponent);
		PlayItemMontage(Montage);
	}

	void OnItemApply() override
	{
		if (!IsValid(PickupTarget))
		{
			return;
		}

		UFVPickupComponent Pickup = UFVPickupComponent::Get(PickupTarget);

		if (IsValid(Pickup))
		{
			Pickup.TryPickup(GetAvatarActorFromActorInfo());
		}
	}

	void OnItemAbilityEnded(bool bWasApplied) override
	{
		PickupTarget = nullptr;
	}
}
