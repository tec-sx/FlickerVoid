// Asks the target's merchant to trade with the instigator, from a dialogue choice or an interaction offer: broadcasts Message.Item.TradeRequested, on which the trade screen (follow-up) opens.
class UFVTradeEffect : UFVScriptEffect
{
	UFUNCTION(BlueprintOverride)
	void Apply(FFVConditionContext Context) const
	{
		UFVMerchantComponent Merchant = Context.Target != nullptr ? UFVMerchantComponent::Get(Context.Target) : nullptr;

		if (IsValid(Merchant))
		{
			Merchant.RequestTrade(Context.Instigator);
		}
	}

	UFUNCTION(BlueprintOverride)
	FText GetDescription() const
	{
		return FText::FromString("Trade with the target");
	}
}
