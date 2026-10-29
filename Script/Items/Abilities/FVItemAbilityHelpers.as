// Montage, motion warping and held item helpers shared by the item abilities; hand props and slots are in FVItem:: (C++).
namespace FVItemAnim
{
	// Montage task on the ability's avatar; none without a montage. Bind its events, then call ReadyForActivation.
	UAbilityTask_PlayMontageAndWait PlayMontage(UGameplayAbility Ability, UAnimMontage Montage)
	{
		if (Ability == nullptr || Montage == nullptr)
		{
			return nullptr;
		}

		return AngelscriptAbilityTask::PlayMontageAndWait(Ability, n"ItemMontage", Montage);
	}

	// Waits once for a montage notify event (Event.Item.Notify.*) on the avatar. Bind EventReceived, then call ReadyForActivation.
	UAbilityTask_WaitGameplayEvent WaitNotify(UGameplayAbility Ability, FGameplayTag NotifyTag)
	{
		// Check: AngelscriptAbilityTask::WaitGameplayEvent mirrors UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, EventTag, OptionalExternalTarget, OnlyTriggerOnce, OnlyMatchExact).
		return AngelscriptAbilityTask::WaitGameplayEvent(Ability, NotifyTag, nullptr, true, true);
	}

	void SetWarpTarget(AActor Character, FName WarpName, USceneComponent Target)
	{
		if (Character == nullptr || Target == nullptr)
		{
			return;
		}

		UMotionWarpingComponent Warping = UMotionWarpingComponent::Get(Character);

		if (IsValid(Warping))
		{
			Warping.AddOrUpdateWarpTargetFromComponent(WarpName, Target, NAME_None, false);
		}
	}

	void ClearWarpTarget(AActor Character, FName WarpName)
	{
		if (Character == nullptr)
		{
			return;
		}

		UMotionWarpingComponent Warping = UMotionWarpingComponent::Get(Character);

		if (IsValid(Warping))
		{
			Warping.RemoveWarpTarget(WarpName);
		}
	}

	// Hand slot the stack is held in; invalid if it isn't held.
	FGameplayTag GetHoldingHand(UFVEquipmentComponent Equipment, FGuid InstanceId)
	{
		if (!IsValid(Equipment) || !InstanceId.IsValid())
		{
			return FGameplayTag();
		}

		if (Equipment.GetEquippedInstance(FVItem::GetHandSlot(true)) == InstanceId)
		{
			return FVItem::GetHandSlot(true);
		}

		if (Equipment.GetEquippedInstance(FVItem::GetHandSlot(false)) == InstanceId)
		{
			return FVItem::GetHandSlot(false);
		}

		return FGameplayTag();
	}

	// Puts away (or takes out again) what the character holds while an action needs both hands.
	void SetHandsBusy(AActor Character, bool bBusy)
	{
		UFVHeldItemComponent Held = Character != nullptr ? UFVHeldItemComponent::Get(Character) : nullptr;

		if (!IsValid(Held))
		{
			return;
		}

		if (bBusy)
		{
			Held.PutAway();
		}
		else
		{
			Held.TakeOut();
		}
	}
}

// World side helpers of the item abilities: the target of an item event, item sounds, dropping and physics.
namespace FVItemWorld
{
	// The payload's world actor; without a payload (events from interaction offers) the owner of the targeted interactable.
	AActor GetWorldTarget(UFVInteractAbility Ability, UFVItemEventPayload Payload)
	{
		if (Payload != nullptr)
		{
			return Payload.WorldActor;
		}

		UFVInteractableComponent Interactable = Ability != nullptr ? Ability.GetInteractable() : nullptr;

		return IsValid(Interactable) ? Interactable.GetOwner() : nullptr;
	}

	void PlaySound(TSoftObjectPtr<USoundBase> Sound, FVector Location)
	{
		USoundBase Loaded = FVItem::LoadSound(Sound);

		if (Loaded != nullptr)
		{
			Gameplay::PlaySoundAtLocation(Loaded, Location, FRotator::ZeroRotator);
		}
	}

	// Item of the world actor's pickup; none if it has no pickup or nothing is left.
	UFVItemDefinition GetWorldItem(AActor WorldActor)
	{
		UFVPickupComponent Pickup = IsValid(WorldActor) ? UFVPickupComponent::Get(WorldActor) : nullptr;

		return IsValid(Pickup) && !Pickup.IsDepleted() ? Pickup.GetItem() : nullptr;
	}

	// Newest stack of the item that isn't worn; invalid if there is none.
	FGuid FindUnwornStack(AActor Owner, UFVItemDefinition Item)
	{
		UFVInventoryComponent Inventory = Owner != nullptr ? UFVInventoryComponent::Get(Owner) : nullptr;

		if (!IsValid(Inventory) || Item == nullptr)
		{
			return FGuid();
		}

		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Owner);
		TArray<FFVItemStack> Stacks = Inventory.GetItems();

		for (int Index = Stacks.Num() - 1; Index >= 0; Index--)
		{
			if (Stacks[Index].Item != Item)
			{
				continue;
			}

			if (IsValid(Equipment) && Equipment.IsInstanceEquipped(Stacks[Index].InstanceId))
			{
				continue;
			}

			return Stacks[Index].InstanceId;
		}

		return FGuid();
	}

	// Picks the world actor's item up into the picker's inventory. The id of the stack now holding it; invalid if nothing was taken.
	FGuid PickUpStack(AActor Picker, AActor WorldActor)
	{
		UFVPickupComponent Pickup = IsValid(WorldActor) ? UFVPickupComponent::Get(WorldActor) : nullptr;

		if (!IsValid(Pickup) || Picker == nullptr)
		{
			return FGuid();
		}

		UFVItemDefinition Item = Pickup.GetItem();

		if (Item == nullptr || !Pickup.TryPickup(Picker))
		{
			return FGuid();
		}

		return FindUnwornStack(Picker, Item);
	}

	// On the ground, Drop Distance (Items settings) in front of the character.
	FTransform GetDropTransform(AActor Character)
	{
		FVector Location = Character.GetActorLocation() + Character.GetActorForwardVector() * FVItem::GetSettings().DropDistance;
		UCapsuleComponent Capsule = Cast<UCapsuleComponent>(Character.RootComponent);

		if (Capsule != nullptr)
		{
			Location.Z -= Capsule.GetScaledCapsuleHalfHeight();
		}

		Location.Z += 20.0;

		// Check: FTransform binds the (FRotator Rotation, FVector Translation) constructor.
		return FTransform(FRotator(0.0, Character.GetActorRotation().Yaw, 0.0), Location);
	}

	// Lets a dropped or thrown world actor fall and roll; returns its simulating root.
	UPrimitiveComponent EnablePhysics(AActor WorldActor)
	{
		if (!IsValid(WorldActor))
		{
			return nullptr;
		}

		AFVWorldItem WorldItem = Cast<AFVWorldItem>(WorldActor);

		if (WorldItem != nullptr)
		{
			WorldItem.bSimulatePhysics = true;
		}

		UPrimitiveComponent Body = Cast<UPrimitiveComponent>(WorldActor.RootComponent);

		if (Body == nullptr)
		{
			return nullptr;
		}

		Body.SetMobility(EComponentMobility::Movable);
		Body.SetCollisionProfileName(CollisionProfile::PhysicsActor);
		Body.SetSimulatePhysics(true);

		// The profile reset every channel, including the interaction channel the item is detected on.
		UFVInteractableComponent Interactable = UFVInteractableComponent::Get(WorldActor);
		if (Interactable != nullptr)
		{
			Interactable.RefreshDetection();
		}

		return Body;
	}
}
