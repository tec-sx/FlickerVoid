// The instigator buys units of an item from the target's merchant stock, e.g. a drink ordered in dialogue without the trade screen. Buys what the stock and the instigator's money allow, up to Quantity; gate it with Has Money.
class UFVBuyItemEffect : UFVEffect
{
	UPROPERTY(EditAnywhere, Category = "Effect")
	UFVItemDefinition Item;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int Quantity = 1;

	UFUNCTION(BlueprintOverride)
	void Apply(FFVConditionContext Context) const
	{
		UFVMerchantComponent Merchant = Context.Target != nullptr ? UFVMerchantComponent::Get(Context.Target) : nullptr;

		if (IsValid(Merchant))
		{
			Merchant.BuyItem(Context.Instigator, Item, Quantity);
		}
	}

	UFUNCTION(BlueprintOverride)
	FText GetDescription() const
	{
		FString ItemName = Item != nullptr ? Item.Display.Name.ToString() : "nothing";

		return FText::FromString("Buy " + Quantity + " x " + ItemName);
	}
}
