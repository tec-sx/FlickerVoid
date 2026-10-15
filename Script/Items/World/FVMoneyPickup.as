// Coins or a wallet lying in the world; taking it (pickup ability) adds Amount to the collector's money.
class AFVMoneyPickup : AActor
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
	UFVSaveableComponent Saveable;

	UPROPERTY(EditAnywhere, Category = "Money", meta = (ClampMin = 1))
	int Amount = 10;

	UPROPERTY(EditAnywhere, Category = "Money")
	USoundBase PickupSound;

	UPROPERTY(SaveGame)
	bool bCollected = false;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		Saveable.OnActorDataLoaded.AddUFunction(this, n"RefreshPresence");
		RefreshPresence();
	}

	// Adds Amount to the collector's inventory money and removes the pickup. False if already collected or the collector has no inventory.
	UFUNCTION()
	bool Collect(AActor Collector)
	{
		UFVInventoryComponent Inventory = Collector != nullptr ? UFVInventoryComponent::Get(Collector) : nullptr;

		if (bCollected || !IsValid(Inventory))
		{
			return false;
		}

		Inventory.AddMoney(Amount);

		if (PickupSound != nullptr)
		{
			Gameplay::PlaySoundAtLocation(PickupSound, GetActorLocation(), FRotator::ZeroRotator);
		}

		// A placed pickup has a save id: it stays as a hidden record so a reload keeps it collected.
		if (Saveable.GetSaveId().IsValid())
		{
			bCollected = true;
			RefreshPresence();
		}
		else
		{
			DestroyActor();
		}

		return true;
	}

	UFUNCTION()
	private void RefreshPresence()
	{
		SetActorHiddenInGame(bCollected);
		SetActorEnableCollision(!bCollected);
	}
}
