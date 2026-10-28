// Steals from the targeted character (the payload's Other Actor, else the interactable's owner) once Steal Check succeeds: some of their money, or one unit of a random stack they don't wear, marked stolen from their faction (else from them). A failed attempt is noticed and raises the faction's notoriety. Both report Message.Item.Stolen.
class UFVPickpocketAbility : UFVInteractAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Pickpocket", meta = (ToolTip = "Rolled with the thief as instigator and the victim as target. None: every attempt succeeds."))
	UFVCheckDefinition StealCheck;

	UPROPERTY(EditDefaultsOnly, Category = "Pickpocket", meta = (ToolTip = "Total the Steal Check must reach."))
	int Difficulty = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Pickpocket", meta = (ClampMin = 0, ToolTip = "Notoriety gained with the victim's faction when an attempt is noticed."))
	int NoticedNotoriety = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Pickpocket", meta = (ClampMin = 1, ClampMax = 100, ToolTip = "Least share of the victim's money taken, in percent; up to all of it."))
	int MinMoneyPercent = 30;

	default TriggerEventTag = GameplayTags::Event_Item_Pickpocket;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		Pickpocket(IsValid(Interactable) ? Interactable.GetOwner() : nullptr);
		EndAbility();
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		UFVItemEventPayload Payload = FVItem::GetItemPayload(EventData);
		AActor Victim = Payload != nullptr && Payload.OtherActor != nullptr ? Payload.OtherActor : FVItemWorld::GetWorldTarget(this, Payload);

		Pickpocket(Victim);
		EndAbility();
	}

	private void Pickpocket(AActor Victim)
	{
		AActor Thief = GetAvatarActorFromActorInfo();
		UFVInventoryComponent ThiefInventory = UFVInventoryComponent::Get(Thief);
		UFVInventoryComponent VictimInventory = IsValid(Victim) && Victim != Thief ? UFVInventoryComponent::Get(Victim) : nullptr;

		if (!IsValid(ThiefInventory) || !IsValid(VictimInventory))
		{
			return;
		}

		FFVItemStolenMessage Message;
		Message.Thief = Thief;
		Message.Victim = Victim;
		Message.Owner = GetOwnerTag(Victim);

		if (StealCheck != nullptr && !FVCheck::RollCheck(StealCheck, Thief, Victim, Difficulty).bSuccess)
		{
			Message.bNoticed = true;
			RaiseNotoriety(Victim);
			UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_Stolen, Message);
			return;
		}

		TArray<FGuid> Stealable = FindStealableStacks(VictimInventory, ThiefInventory, Victim);
		bool bTakeMoney = VictimInventory.GetMoney() > 0 && (Stealable.Num() == 0 || Math::RandRange(0, 1) == 0);

		if (bTakeMoney)
		{
			Message.Money = StealMoney(VictimInventory, ThiefInventory);
		}
		else if (Stealable.Num() > 0)
		{
			Message.Item = StealItem(VictimInventory, ThiefInventory, Stealable[Math::RandRange(0, Stealable.Num() - 1)], Message.Owner);
		}

		// Empty pockets: nothing to report.
		if (Message.Money > 0 || Message.Item != nullptr)
		{
			UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_Stolen, Message);
		}
	}

	// Stacks the victim doesn't wear, without quest items, that the thief can carry.
	private TArray<FGuid> FindStealableStacks(UFVInventoryComponent From, UFVInventoryComponent To, AActor Victim)
	{
		TArray<FGuid> Stealable;
		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Victim);
		TArray<FFVItemStack> Stacks = From.GetItems();

		for (FFVItemStack Stack : Stacks)
		{
			if (Stack.Item == nullptr || Stack.Item.bQuestItem || !To.CanAddItem(Stack.Item, 1))
			{
				continue;
			}

			if (IsValid(Equipment) && Equipment.IsInstanceEquipped(Stack.InstanceId))
			{
				continue;
			}

			Stealable.Add(Stack.InstanceId);
		}

		return Stealable;
	}

	private int StealMoney(UFVInventoryComponent From, UFVInventoryComponent To)
	{
		int Amount = Math::IntegerDivisionTrunc(Math::Max(1, From.GetMoney() * Math::RandRange(MinMoneyPercent, 100)), 100);

		return From.TransferMoney(To, Amount);
	}

	// One unit with the Stolen mark; none if it couldn't be moved.
	private UFVItemDefinition StealItem(UFVInventoryComponent From, UFVInventoryComponent To, FGuid InstanceId, FGameplayTag Owner)
	{
		FFVItemStack Stack;

		if (!From.GetStack(InstanceId, Stack) || From.TransferStackAsStolen(To, InstanceId, 1, Owner) <= 0)
		{
			return nullptr;
		}

		return Stack.Item;
	}

	// The victim's faction Id, else their character Id; empty if they have neither.
	private FGameplayTag GetOwnerTag(AActor Victim)
	{
		UFVFactionDefinition Faction = FVSocial::GetActorFaction(Victim);

		if (Faction != nullptr && Faction.Id.IsValid())
		{
			return Faction.Id;
		}

		UFVCharacterDefinition Character = UFVIdentityComponent::GetActorDefinition(Victim);

		return Character != nullptr ? Character.Id : FGameplayTag();
	}

	private void RaiseNotoriety(AActor Victim)
	{
		UFVFactionDefinition Faction = FVSocial::GetActorFaction(Victim);

		if (Faction != nullptr && NoticedNotoriety > 0)
		{
			FVSocial::ModifyNotoriety(Faction, NoticedNotoriety);
		}
	}
}
