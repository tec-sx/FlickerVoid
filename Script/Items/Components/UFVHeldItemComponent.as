// Shows what the owner holds in its hand slots (Equipment.Slot.Hand.Right/Left): the item's World actor class, or a mesh prop, at the hand socket, and the hold poses on the player's anim instance. Abilities that need both hands put held items away.
class UFVHeldItemComponent : UActorComponent
{
	private UFVEquipmentComponent Equipment;
	private AActor RightActor;
	private AActor LeftActor;
	private UStaticMeshComponent RightProp;
	private UStaticMeshComponent LeftProp;
	private bool bPutAway = false;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		Equipment = UFVEquipmentComponent::Get(GetOwner());

		if (!IsValid(Equipment))
		{
			return;
		}

		Equipment.OnEquipmentChanged.AddUFunction(this, n"HandleEquipmentChanged");
		Equipment.OnEquipmentReset.AddUFunction(this, n"HandleEquipmentReset");

		RefreshHand(true);
		RefreshHand(false);
	}

	UFUNCTION(BlueprintOverride)
	void EndPlay(EEndPlayReason EndPlayReason)
	{
		ClearHand(true);
		ClearHand(false);
	}

	UFUNCTION(meta = (ToolTip = "Hides the held items and clears the hold poses while an action needs both hands."))
	void PutAway()
	{
		bPutAway = true;
		RefreshPresentation();
	}

	UFUNCTION(meta = (ToolTip = "Shows the held items again after Put Away."))
	void TakeOut()
	{
		bPutAway = false;
		RefreshPresentation();
	}

	UFUNCTION(meta = (ToolTip = "Uses the item held in that hand: its held actor's Held Item Behaviour component, else Event.Item.Use for a usable item. False if nothing happened."))
	bool UseHeld(bool bRightHand)
	{
		if (!IsValid(Equipment) || bPutAway)
		{
			return false;
		}

		AActor HeldActor = bRightHand ? RightActor : LeftActor;
		UFVHeldItemBehaviourComponent Behaviour = IsValid(HeldActor) ? UFVHeldItemBehaviourComponent::Get(HeldActor) : nullptr;

		if (IsValid(Behaviour))
		{
			Behaviour.Use(GetOwner());
			return true;
		}

		FGameplayTag Slot = FVItemAnim::GetHandSlot(bRightHand);
		UFVItemDefinition Item = Equipment.GetEquipped(Slot);

		if (Item == nullptr || !Item.IsUsable())
		{
			return false;
		}

		return FVItem::SendItemEvent(GetOwner(), GameplayTags::Event_Item_Use, Item, Equipment.GetEquippedInstance(Slot), nullptr, nullptr, 1);
	}

	UFUNCTION()
	private void HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
	{
		if (Slot == FVItemAnim::GetHandSlot(true))
		{
			RefreshHand(true);
		}
		else if (Slot == FVItemAnim::GetHandSlot(false))
		{
			RefreshHand(false);
		}
	}

	UFUNCTION()
	private void HandleEquipmentReset()
	{
		RefreshHand(true);
		RefreshHand(false);
	}

	private void RefreshHand(bool bRightHand)
	{
		ClearHand(bRightHand);

		FGameplayTag Slot = FVItemAnim::GetHandSlot(bRightHand);
		UFVItemDefinition Item = Equipment.GetEquipped(Slot);
		USkeletalMeshComponent CharacterMesh = FVItemAnim::GetCharacterMesh(GetOwner());

		if (Item != nullptr && CharacterMesh != nullptr)
		{
			FName Socket = FVItem::GetHandSocket(bRightHand);
			AActor HeldActor = SpawnHeldActor(Item, Slot, CharacterMesh, Socket);

			if (HeldActor != nullptr)
			{
				SetHand(bRightHand, HeldActor, nullptr);
			}
			else
			{
				SetHand(bRightHand, nullptr, FVItemAnim::AttachProp(GetOwner(), Item, Socket));
			}
		}

		RefreshPresentation();
	}

	// The World fragment's Actor Class, spawned with the held unit on its pickup and made inert (no collision, physics or offers); none without one.
	private AActor SpawnHeldActor(UFVItemDefinition Item, FGameplayTag Slot, USkeletalMeshComponent CharacterMesh, FName Socket)
	{
		FFVItemFragment_World World;

		// Check: TSoftClassPtr binds IsNull().
		if (!FVItem::GetWorldFragment(Item, World) || World.ActorClass.IsNull())
		{
			return nullptr;
		}

		FFVItemStack Unit;
		UFVInventoryComponent Inventory = UFVInventoryComponent::Get(GetOwner());

		if (!IsValid(Inventory) || !Inventory.GetStack(Equipment.GetEquippedInstance(Slot), Unit))
		{
			Unit.Item = Item;
		}

		Unit.Quantity = 1;

		AActor HeldActor = FVItem::SpawnWorldItem(Unit, CharacterMesh.GetSocketTransform(Socket));

		if (HeldActor == nullptr)
		{
			return nullptr;
		}

		HeldActor.SetActorEnableCollision(false);

		UFVInteractableComponent Offers = UFVInteractableComponent::Get(HeldActor);

		if (IsValid(Offers))
		{
			Offers.DeactivateInteractions();
		}

		USceneComponent Root = HeldActor.RootComponent;
		UPrimitiveComponent Body = Cast<UPrimitiveComponent>(Root);

		if (Body != nullptr)
		{
			Body.SetSimulatePhysics(false);
		}

		Root.AttachToComponent(
			CharacterMesh,
			Socket,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepRelative,
			false);
		Root.SetRelativeTransform(FVItemAnim::GetGripTransform(Item, Cast<UStaticMeshComponent>(Root)));

		return HeldActor;
	}

	private void SetHand(bool bRightHand, AActor HeldActor, UStaticMeshComponent Prop)
	{
		if (bRightHand)
		{
			RightActor = HeldActor;
			RightProp = Prop;
		}
		else
		{
			LeftActor = HeldActor;
			LeftProp = Prop;
		}
	}

	private void ClearHand(bool bRightHand)
	{
		AActor HeldActor = bRightHand ? RightActor : LeftActor;

		if (IsValid(HeldActor))
		{
			HeldActor.DestroyActor();
		}

		FVItemAnim::DetachProp(bRightHand ? RightProp : LeftProp);
		SetHand(bRightHand, nullptr, nullptr);
	}

	// Visibility of the held items and the hold poses, following Put Away.
	private void RefreshPresentation()
	{
		SetVisible(RightActor, RightProp);
		SetVisible(LeftActor, LeftProp);

		USkeletalMeshComponent CharacterMesh = FVItemAnim::GetCharacterMesh(GetOwner());
		UFVPlayerAnimInstance AnimInstance = CharacterMesh != nullptr ? Cast<UFVPlayerAnimInstance>(CharacterMesh.AnimInstance) : nullptr;

		if (AnimInstance != nullptr)
		{
			AnimInstance.RightHandHoldPose = FindHoldPose(true);
			AnimInstance.LeftHandHoldPose = FindHoldPose(false);
		}
	}

	private void SetVisible(AActor HeldActor, UStaticMeshComponent Prop)
	{
		if (IsValid(HeldActor))
		{
			HeldActor.SetActorHiddenInGame(bPutAway);
		}

		if (IsValid(Prop))
		{
			Prop.SetVisibility(!bPutAway);
		}
	}

	private UAnimSequenceBase FindHoldPose(bool bRightHand)
	{
		if (bPutAway || !IsValid(Equipment))
		{
			return nullptr;
		}

		UFVItemDefinition Item = Equipment.GetEquipped(FVItemAnim::GetHandSlot(bRightHand));

		return Item != nullptr ? FVItem::FindHoldPose(GetOwner(), Item) : nullptr;
	}
}
