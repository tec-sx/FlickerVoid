// An item lying in the world. The stack sits on the pickup; the mesh and the offers come from the item definition (World fragment, else the Items settings).
class AFVWorldItem : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	UStaticMeshComponent Mesh;
	default Mesh.ComponentTags.Add(n"Detectable");

	UPROPERTY(DefaultComponent)
	UFVInteractableComponent Interactable;

	UPROPERTY(DefaultComponent)
	UFVInteractableHighlightComponent Highlight;
	default Highlight.HighlightSetup.HighlightType = EFVHighlightType::PostProcessing;
	default Highlight.HighlightSetup.StencilID = 133;

	UPROPERTY(DefaultComponent)
	UFVPickupComponent Pickup;
	default Pickup.bDestroyOwnerOnPickup = false;

	UPROPERTY(DefaultComponent)
	UFVSaveableComponent Saveable;

	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ToolTip = "Item placed here, given to the pickup. Dropped and thrown items get their stack from the spawner instead."))
	UFVItemDefinition Item;

	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ClampMin = 1))
	int Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (ToolTip = "The item is only there while these pass, with the player as instigator; checked again when facts change."))
	FFVConditionSet AppearConditions;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (ToolTip = "Falls and rolls as a physics body."))
	bool bSimulatePhysics = false;

	UPROPERTY(SaveGame)
	bool bCollected = false;

	private bool bPresent = true;

	UFUNCTION(BlueprintOverride)
	void ConstructionScript()
	{
		if (Item != nullptr)
		{
			FFVItemStack PlacedStack = Pickup.GetStack();
			PlacedStack.Item = Item;
			PlacedStack.Quantity = Quantity;
			Pickup.SetStack(PlacedStack);
		}

		ApplyItemLooks();

		if (bSimulatePhysics)
		{
			Mesh.SetCollisionProfileName(CollisionProfile::PhysicsActor);
			Mesh.SetSimulatePhysics(true);
		}
	}

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		Pickup.OnPickedUp.AddUFunction(this, n"OnPickedUp");
		Saveable.OnActorDataLoaded.AddUFunction(this, n"RefreshPresence");

		UFVFactDatabase Facts = UFVFactDatabase::Get();

		if (Facts != nullptr)
		{
			Facts.OnFactsChanged.AddUFunction(this, n"RefreshPresence");
		}

		RefreshPresence();
	}

	UFUNCTION(BlueprintOverride)
	void EndPlay(EEndPlayReason EndPlayReason)
	{
		UFVFactDatabase Facts = UFVFactDatabase::Get();

		if (Facts != nullptr)
		{
			Facts.OnFactsChanged.UnbindObject(this);
		}
	}

	UFUNCTION(BlueprintPure)
	FFVItemStack GetStack() const
	{
		return Pickup.GetStack();
	}

	// Takes one unit off the stack, e.g. after it was used straight from the world; the last one removes the item like a pickup.
	UFUNCTION()
	void ConsumeOne()
	{
		FFVItemStack Remaining = Pickup.GetStack();

		if (Remaining.Quantity <= 0)
		{
			return;
		}

		Remaining.Quantity -= 1;
		Pickup.SetStack(Remaining);

		if (Pickup.IsDepleted())
		{
			RemoveFromWorld();
		}
	}

	UFUNCTION()
	private void OnPickedUp(AActor Picker)
	{
		FFVItemFragment_World WorldFragment;

		if (FVItem::GetWorldFragment(Pickup.GetItem(), WorldFragment))
		{
			FVItemWorld::PlaySound(WorldFragment.PickupSound, GetActorLocation());
		}

		if (Pickup.IsDepleted())
		{
			RemoveFromWorld();
		}
	}

	UFUNCTION()
	private void RefreshPresence()
	{
		bool bShouldBePresent = !bCollected
			&& !Pickup.IsDepleted()
			&& FVCondition::EvaluateConditionSet(AppearConditions, Gameplay::GetPlayerPawn(0), this);

		if (bShouldBePresent == bPresent)
		{
			return;
		}

		bPresent = bShouldBePresent;
		SetActorHiddenInGame(!bPresent);
		SetActorEnableCollision(bPresent);

		if (bSimulatePhysics)
		{
			Mesh.SetSimulatePhysics(bPresent);
		}
	}

	// A placed item has a save id: it stays as a hidden record so a reload keeps it collected. A spawned one is destroyed.
	private void RemoveFromWorld()
	{
		if (Saveable.GetSaveId().IsValid())
		{
			bCollected = true;
			RefreshPresence();
			return;
		}

		DestroyActor();
	}

	// The item's world mesh and offers replace the previous ones, so changing the item in the editor updates them.
	private void ApplyItemLooks()
	{
		UFVItemDefinition StackItem = Pickup.GetItem();

		if (StackItem == nullptr)
		{
			return;
		}

		UStaticMesh ItemMesh = FVItem::LoadWorldMesh(StackItem);

		if (ItemMesh != nullptr)
		{
			Mesh.SetStaticMesh(ItemMesh);
		}

		UFVInteractableDefinition Offers = FVItem::FindInteractable(StackItem);

		if (Offers != nullptr)
		{
			Interactable.Definition = Offers;
		}
	}
}
