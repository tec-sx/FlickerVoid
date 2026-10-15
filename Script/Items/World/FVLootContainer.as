// A crate, drawer or body holding items and money. The loot table rolls on the first opening; a lock (Lock component added on the instance or a Blueprint) opens with a carried key. Contents with an owner are marked stolen when taken.
class AFVLootContainer : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	UStaticMeshComponent Mesh;
	default Mesh.ComponentTags.Add(n"Detectable");

	UPROPERTY(DefaultComponent, Attach = Mesh, meta = (ToolTip = "Where the character reaches to (motion warping target of item montages)."))
	USceneComponent InteractionPoint;

	UPROPERTY(DefaultComponent)
	UFVInteractableComponent Interactable;

	UPROPERTY(DefaultComponent)
	UFVInteractableHighlightComponent Highlight;
	default Highlight.HighlightSetup.HighlightType = EFVHighlightType::PostProcessing;
	default Highlight.HighlightSetup.StencilID = 133;

	UPROPERTY(DefaultComponent)
	UFVSaveableComponent Saveable;

	UPROPERTY(DefaultComponent)
	UFVInventoryComponent Storage;
	default Storage.bApplyCarryLimits = false;

	UPROPERTY(DefaultComponent)
	UFVLootComponent Loot;

	// Unlocks with a carried key if locked, rolls the loot once and broadcasts Message.Item.ContainerOpened (the take screen opens on it). False if it stays locked.
	UFUNCTION()
	bool TryOpen(AActor Opener)
	{
		if (!UnlockAndFill(Opener))
		{
			return false;
		}

		FFVItemContainerMessage Message;
		Message.Opener = Opener;
		Message.Container = this;

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_ContainerOpened, Message);

		return true;
	}

	// Opens without the take screen and moves everything, money included, to the taker's inventory. Owned contents get the Stolen mark and a Message.Item.Stolen per item taken. Returns the units taken.
	UFUNCTION()
	int TakeAll(AActor Taker)
	{
		UFVInventoryComponent TakerInventory = Taker != nullptr ? UFVInventoryComponent::Get(Taker) : nullptr;

		if (!IsValid(TakerInventory) || !UnlockAndFill(Taker))
		{
			return 0;
		}

		TArray<FFVItemStack> Before = Storage.GetItems();
		int MoneyBefore = Storage.GetMoney();

		Storage.TransferAll(TakerInventory, true, Loot.GetOwnerTag());

		bool bOwned = Loot.GetOwnerTag().IsValid();
		int MoneyToReport = MoneyBefore - Storage.GetMoney();
		int UnitsTaken = 0;

		for (FFVItemStack Stack : Before)
		{
			FFVItemStack Left;
			int LeftQuantity = Storage.GetStack(Stack.InstanceId, Left) ? Left.Quantity : 0;
			int Taken = Stack.Quantity - LeftQuantity;

			if (Taken <= 0)
			{
				continue;
			}

			UnitsTaken += Taken;

			if (bOwned)
			{
				ReportStolen(Taker, Stack.Item, MoneyToReport);
				MoneyToReport = 0;
			}
		}

		if (bOwned && MoneyToReport > 0)
		{
			ReportStolen(Taker, nullptr, MoneyToReport);
		}

		return UnitsTaken;
	}

	private bool UnlockAndFill(AActor Opener)
	{
		UFVLockComponent Lock = UFVLockComponent::Get(this);

		if (IsValid(Lock) && Lock.IsLocked() && !FVItemKeys::TryUnlockWithKey(Opener, Lock))
		{
			return false;
		}

		Loot.RollIfNeeded(Opener);

		return true;
	}

	private void ReportStolen(AActor Thief, UFVItemDefinition Item, int Money)
	{
		FFVItemStolenMessage Message;
		Message.Thief = Thief;
		Message.Victim = this;
		Message.Item = Item;
		Message.Money = Money;
		Message.Owner = Loot.GetOwnerTag();
		Message.bNoticed = false;

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_Stolen, Message);
	}
}
