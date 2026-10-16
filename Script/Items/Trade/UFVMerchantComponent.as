// Goes on a merchant next to the inventory holding its stock and money (Apply Carry Limits off). Prices start from the item's Value fragment; the customer's standing with the merchant's faction and a disguise as one of them lower buying prices and raise selling prices, by at most 50%. The trade screen (follow-up) opens on Message.Item.TradeRequested.
class UFVMerchantComponent : UActorComponent
{
	UPROPERTY(EditAnywhere, Category = "Merchant", meta = (ToolTip = "Faction whose standing and disguise change prices. None: the owner's faction (Social fragment)."))
	UFVFactionDefinition Faction;

	UPROPERTY(EditAnywhere, Category = "Merchant", meta = (ClampMin = 0, ToolTip = "Base price multiplier when the customer buys."))
	float BuyMultiplier = 1.0;

	UPROPERTY(EditAnywhere, Category = "Merchant", meta = (ClampMin = 0, ToolTip = "Base price multiplier when the customer sells."))
	float SellMultiplier = 0.5;

	UPROPERTY(EditAnywhere, Category = "Merchant", meta = (ClampMin = 0, ToolTip = "Price change per point of the customer's standing with the faction: positive standing lowers buying prices and raises selling prices, negative does the opposite."))
	float StandingPriceFactor = 0.005;

	UPROPERTY(EditAnywhere, Category = "Merchant", meta = (ClampMin = 0, ClampMax = 0.5, ToolTip = "Extra price change for a customer disguised as one of the faction."))
	float DisguiseDiscount = 0.1;

	UPROPERTY(EditAnywhere, Category = "Merchant", meta = (ToolTip = "Buys goods stolen from its own faction or from itself, which other merchants refuse."))
	bool bFence = false;

	UFUNCTION(BlueprintPure, meta = (ToolTip = "The owner's inventory: stock and money."))
	UFVInventoryComponent GetStock() const
	{
		return UFVInventoryComponent::Get(GetOwner());
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Faction, else the owner's faction; none if it has neither."))
	UFVFactionDefinition GetPriceFaction() const
	{
		return Faction != nullptr ? Faction : FVSocial::GetActorFaction(GetOwner());
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Price of one unit for the buyer; 0 for an item without a base price."))
	int GetBuyPrice(UFVItemDefinition Item, AActor Buyer) const
	{
		int BasePrice = FVItemStack::GetBasePrice(Item);

		if (BasePrice <= 0)
		{
			return 0;
		}

		return Math::Max(1, Math::RoundToInt(BasePrice * BuyMultiplier * (1.0 - GetPriceAdjustment(Buyer))));
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "What the merchant pays for one unit of the stack; 0 when it won't buy it: not sellable, a quest item, or stolen from its faction or from itself (unless it is a fence)."))
	int GetSellPrice(FFVItemStack Stack, AActor Seller) const
	{
		if (Stack.Item == nullptr || Stack.Item.bQuestItem || !FVItemStack::IsSellable(Stack.Item) || IsRefusedAsStolen(Stack))
		{
			return 0;
		}

		return Math::RoundToInt(FVItemStack::GetBasePrice(Stack.Item) * SellMultiplier * (1.0 + GetPriceAdjustment(Seller)));
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "The stock stack holds Quantity units that aren't worn, and the buyer can pay for and carry them."))
	bool CanBuy(AActor Buyer, FGuid StockInstanceId, int Quantity) const
	{
		UFVInventoryComponent StockInventory = GetStock();
		UFVInventoryComponent BuyerInventory = Buyer != nullptr && Buyer != GetOwner() ? UFVInventoryComponent::Get(Buyer) : nullptr;
		FFVItemStack Stack;

		if (!IsValid(StockInventory) || !IsValid(BuyerInventory) || Quantity <= 0 || !StockInventory.GetStack(StockInstanceId, Stack) || Stack.Quantity < Quantity)
		{
			return false;
		}

		if (IsWorn(GetOwner(), StockInstanceId))
		{
			return false;
		}

		return BuyerInventory.CanAfford(GetBuyPrice(Stack.Item, Buyer) * Quantity) && BuyerInventory.CanAddItem(Stack.Item, Quantity);
	}

	UFUNCTION(meta = (ToolTip = "Moves Quantity units of a stock stack to the buyer, who pays for them. False if Can Buy fails or not every unit moved (the units that moved are paid)."))
	bool Buy(AActor Buyer, FGuid StockInstanceId, int Quantity)
	{
		if (!CanBuy(Buyer, StockInstanceId, Quantity))
		{
			return false;
		}

		UFVInventoryComponent StockInventory = GetStock();
		UFVInventoryComponent BuyerInventory = UFVInventoryComponent::Get(Buyer);
		FFVItemStack Stack;
		StockInventory.GetStack(StockInstanceId, Stack);

		int UnitPrice = GetBuyPrice(Stack.Item, Buyer);
		int Bought = StockInventory.TransferStack(BuyerInventory, StockInstanceId, Quantity);

		if (Bought > 0)
		{
			BuyerInventory.TransferMoney(StockInventory, UnitPrice * Bought);
		}

		return Bought == Quantity;
	}

	UFUNCTION(meta = (ToolTip = "Buys up to Quantity units of the item, stack by stack, e.g. a drink ordered in dialogue. Returns the units bought."))
	int BuyItem(AActor Buyer, UFVItemDefinition Item, int Quantity)
	{
		UFVInventoryComponent StockInventory = GetStock();

		if (!IsValid(StockInventory) || Item == nullptr)
		{
			return 0;
		}

		int Bought = 0;
		TArray<FFVItemStack> Stacks = StockInventory.GetItems();

		for (FFVItemStack Stack : Stacks)
		{
			if (Bought >= Quantity)
			{
				break;
			}

			if (Stack.Item != Item)
			{
				continue;
			}

			int Wanted = Math::Min(Quantity - Bought, Stack.Quantity);

			if (Buy(Buyer, Stack.InstanceId, Wanted))
			{
				Bought += Wanted;
			}
		}

		return Bought;
	}

	UFUNCTION(BlueprintPure, meta = (ToolTip = "The seller's stack holds Quantity units that aren't worn, the merchant buys it and can pay for them."))
	bool CanSell(AActor Seller, FGuid InstanceId, int Quantity) const
	{
		UFVInventoryComponent StockInventory = GetStock();
		UFVInventoryComponent SellerInventory = Seller != nullptr && Seller != GetOwner() ? UFVInventoryComponent::Get(Seller) : nullptr;
		FFVItemStack Stack;

		if (!IsValid(StockInventory) || !IsValid(SellerInventory) || Quantity <= 0 || !SellerInventory.GetStack(InstanceId, Stack) || Stack.Quantity < Quantity)
		{
			return false;
		}

		if (IsWorn(Seller, InstanceId))
		{
			return false;
		}

		int UnitPrice = GetSellPrice(Stack, Seller);

		return UnitPrice > 0 && StockInventory.CanAfford(UnitPrice * Quantity);
	}

	UFUNCTION(meta = (ToolTip = "Moves Quantity units of the seller's stack into the stock, keeping a Stolen mark, and pays for them. False if Can Sell fails or not every unit moved (the units that moved are paid)."))
	bool Sell(AActor Seller, FGuid InstanceId, int Quantity)
	{
		if (!CanSell(Seller, InstanceId, Quantity))
		{
			return false;
		}

		UFVInventoryComponent StockInventory = GetStock();
		UFVInventoryComponent SellerInventory = UFVInventoryComponent::Get(Seller);
		FFVItemStack Stack;
		SellerInventory.GetStack(InstanceId, Stack);

		int UnitPrice = GetSellPrice(Stack, Seller);
		int Sold = SellerInventory.TransferStack(StockInventory, InstanceId, Quantity);

		if (Sold > 0)
		{
			StockInventory.TransferMoney(SellerInventory, UnitPrice * Sold);
		}

		return Sold == Quantity;
	}

	UFUNCTION(meta = (ToolTip = "Broadcasts Message.Item.TradeRequested with the customer as Opener and the merchant as Container; the trade screen opens on it."))
	void RequestTrade(AActor Customer)
	{
		FFVItemContainerMessage Message;
		Message.Opener = Customer;
		Message.Container = GetOwner();

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Message_Item_TradeRequested, Message);
	}

	// Share taken off buying prices and added to selling prices, within plus or minus 50%.
	private float GetPriceAdjustment(AActor Customer) const
	{
		UFVFactionDefinition MerchantFaction = GetPriceFaction();

		if (MerchantFaction == nullptr)
		{
			return 0.0;
		}

		float Adjustment = FVSocial::GetStanding(MerchantFaction) * StandingPriceFactor;

		if (Customer != nullptr && FVSocial::IsDisguisedAs(Customer, MerchantFaction))
		{
			Adjustment += DisguiseDiscount;
		}

		return Math::Clamp(Adjustment, -0.5, 0.5);
	}

	private bool IsRefusedAsStolen(FFVItemStack Stack) const
	{
		FGameplayTag StolenFrom;

		if (bFence || !FVItemStack::IsStolen(Stack, StolenFrom) || !StolenFrom.IsValid())
		{
			return false;
		}

		UFVFactionDefinition MerchantFaction = GetPriceFaction();

		if (MerchantFaction != nullptr && StolenFrom == MerchantFaction.Id)
		{
			return true;
		}

		UFVCharacterDefinition Character = UFVIdentityComponent::GetActorDefinition(GetOwner());

		return Character != nullptr && StolenFrom == Character.Id;
	}

	private bool IsWorn(AActor Wearer, FGuid InstanceId) const
	{
		UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(Wearer);

		return IsValid(Equipment) && Equipment.IsInstanceEquipped(InstanceId);
	}
}
