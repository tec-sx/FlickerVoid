// Reads a note in the world (AFVReadable): plays the read montage (Animation.Item.Use.Read) and the note is read on the Apply notify.
class UFVReadAbility : UFVItemMontageAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_Read;
	default ActivationOwnedTags.AddTag(GameplayTags::Status_Interacting);
	default ActivationOwnedTags.AddTag(GameplayTags::Status_HandsBusy);

	private AFVReadable Readable;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		StartRead(IsValid(Interactable) ? Interactable.GetOwner() : nullptr);
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		StartRead(FVItemWorld::GetWorldTarget(this, FVItem::GetItemPayload(EventData)));
	}

	private void StartRead(AActor Target)
	{
		Readable = Cast<AFVReadable>(Target);

		if (!IsValid(Readable))
		{
			EndAbility();
			return;
		}

		AActor Avatar = GetAvatarActorFromActorInfo();

		SetItemWarpTarget(Readable.RootComponent);
		PlayItemMontage(FVItem::FindMontage(Avatar, GameplayTags::Animation_Item_Use_Read));
	}

	void OnItemApply() override
	{
		if (IsValid(Readable))
		{
			Readable.MarkRead(GetAvatarActorFromActorInfo());
		}
	}

	void OnItemAbilityEnded(bool bWasApplied) override
	{
		Readable = nullptr;
	}
}
