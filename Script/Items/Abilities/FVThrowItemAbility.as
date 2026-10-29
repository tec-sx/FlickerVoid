// Throws one unit of a throwable item: the payload's stack, else what the character holds (right hand first). The item leaves the hand on the throw montage's Apply notify, flies along the aim and makes noise where it lands. Also bindable to an input tag.
class UFVThrowItemAbility : UFVItemMontageAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Throw", meta = (Units = "Degrees", ToolTip = "Upward angle added to the aim."))
	float AimPitch = 10.0;

	UPROPERTY(EditDefaultsOnly, Category = "Throw", meta = (ClampMin = 0, Units = "cm", ToolTip = "How far in front of the hand the item starts its flight, clear of the thrower."))
	float ReleaseDistance = 30.0;

	default TriggerEventTag = GameplayTags::Event_Item_Throw;
	default ActivationBlockedTags.AddTag(GameplayTags::Status_HandsBusy);

	private FGuid ThrownId;
	private UFVItemDefinition ThrownItem;
	private bool bRightHand = true;
	private bool bFromHand = false;
	private UStaticMeshComponent Prop;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		StartThrow(nullptr);
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		StartThrow(FVItem::GetItemPayload(EventData));
	}

	private void StartThrow(UFVItemEventPayload Payload)
	{
		AActor Avatar = GetAvatarActorFromActorInfo();
		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Avatar);

		ThrownId = Payload != nullptr && Payload.InstanceId.IsValid() ? Payload.InstanceId : FindHeldThrowable(Equipment);
		ThrownItem = nullptr;

		FGameplayTag Hand = FVItemAnim::GetHoldingHand(Equipment, ThrownId);
		bFromHand = Hand.IsValid();
		bRightHand = !bFromHand || Hand == FVItem::GetHandSlot(true);

		if (!CanThrow(Avatar, Equipment))
		{
			EndAbility();
			return;
		}

		PlayItemMontage(FVItem::FindMontage(Avatar, GameplayTags::Animation_Item_Throw));
	}

	private FGuid FindHeldThrowable(UFVEquipmentComponent Equipment)
	{
		if (!IsValid(Equipment))
		{
			return FGuid();
		}

		FFVItemFragment_Throwable Throwable;

		for (int Hand = 0; Hand < 2; Hand++)
		{
			FGameplayTag Slot = FVItem::GetHandSlot(Hand == 0);

			if (FVItem::GetThrowableFragment(Equipment.GetEquipped(Slot), Throwable))
			{
				return Equipment.GetEquippedInstance(Slot);
			}
		}

		return FGuid();
	}

	private bool CanThrow(AActor Avatar, UFVEquipmentComponent Equipment)
	{
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Avatar);
		FFVItemStack Stack;
		FFVItemFragment_Throwable Throwable;

		if (!IsValid(Inventory) || !Inventory.GetStack(ThrownId, Stack) || !FVItem::GetThrowableFragment(Stack.Item, Throwable))
		{
			return false;
		}

		if (Stack.Item.bQuestItem || Stack.Item.IsKeyItem())
		{
			return false;
		}

		// Worn anywhere but in a hand.
		if (!bFromHand && IsValid(Equipment) && Equipment.IsInstanceEquipped(ThrownId))
		{
			return false;
		}

		ThrownItem = Stack.Item;
		return true;
	}

	bool PutsAwayHeldItems() override
	{
		return !bFromHand;
	}

	void OnItemAttach() override
	{
		if (!bFromHand && Prop == nullptr)
		{
			Prop = FVItem::AttachProp(GetAvatarActorFromActorInfo(), ThrownItem, FVItem::GetHandSocket(true));
		}
	}

	void OnItemApply() override
	{
		FVItem::DetachProp(Prop);
		Prop = nullptr;
		Release(GetAvatarActorFromActorInfo());
	}

	void OnItemAbilityEnded(bool bWasApplied) override
	{
		FVItem::DetachProp(Prop);
		Prop = nullptr;
		ThrownItem = nullptr;
	}

	private void Release(AActor Avatar)
	{
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(Avatar);
		FFVItemStack Stack;
		FFVItemFragment_Throwable Throwable;

		if (!IsValid(Inventory) || !Inventory.GetStack(ThrownId, Stack) || !FVItem::GetThrowableFragment(Stack.Item, Throwable))
		{
			return;
		}

		FVector Direction = GetAimDirection(Avatar);
		USkeletalMeshComponent CharacterMesh = FVItem::GetCharacterMesh(Avatar);
		FVector Start = CharacterMesh != nullptr
			? CharacterMesh.GetSocketLocation(FVItem::GetHandSocket(bRightHand))
			: Avatar.GetActorLocation();
		Start += Direction * ReleaseDistance;

		FFVItemStack Unit = Stack;
		Unit.Quantity = 1;

		AActor Thrown = FVItem::SpawnWorldItem(Unit, FTransform(Avatar.GetActorRotation(), Start));

		if (Thrown == nullptr)
		{
			return;
		}

		// A held stack's last unit leaves the hand slot with it.
		if (!Inventory.RemoveStack(ThrownId, 1))
		{
			Thrown.DestroyActor();
			return;
		}

		UPrimitiveComponent Body = FVItemWorld::EnablePhysics(Thrown);

		if (Body != nullptr)
		{
			Body.AddImpulse(Direction * Throwable.ThrowSpeed, NAME_None, true);
		}

		UFVThrownItemComponent Flight = UFVThrownItemComponent::Create(Thrown);
		Flight.Arm(Avatar, Throwable);
	}

	private FVector GetAimDirection(AActor Avatar)
	{
		APawn Pawn = Cast<APawn>(Avatar);
		FRotator Aim = Pawn != nullptr ? Pawn.GetControlRotation() : Avatar.GetActorRotation();

		// Check: FRotator binds ForwardVector (FRotator::Vector()).
		return FRotator(Aim.Pitch + AimPitch, Aim.Yaw, 0.0).ForwardVector;
	}
}
