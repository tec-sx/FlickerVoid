#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVMerchantComponent.generated.h"

class UFVFactionDefinition;
class UFVItemDefinition;

/**
 * Goes on a merchant next to the inventory holding its stock and money (Apply Carry Limits off). Prices start from the
 * item's Value fragment; the customer's standing with the merchant's faction and a disguise as one of them lower buying
 * prices and raise selling prices, by at most 50%. The trade screen opens on Message.Item.TradeRequested.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVMerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVMerchantComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintPure, Category = "FV|Item|Trade", meta = (ToolTip = "The owner's inventory: stock and money."))
	UFVInventoryComponent* GetStock() const;

	UFUNCTION(BlueprintPure, Category = "FV|Item|Trade", meta = (ToolTip = "Faction, else the owner's faction; none if it has neither."))
	UFVFactionDefinition* GetPriceFaction() const;

	UFUNCTION(BlueprintPure, Category = "FV|Item|Trade", meta = (ToolTip = "Price of one unit for the buyer; 0 for an item without a base price."))
	int32 GetBuyPrice(const UFVItemDefinition* Item, const AActor* Buyer) const;

	UFUNCTION(BlueprintPure, Category = "FV|Item|Trade", meta = (ToolTip = "What the merchant pays for one unit of the stack; 0 when it won't buy it: not sellable, a quest item, or stolen from its faction or from itself (unless it is a fence)."))
	int32 GetSellPrice(const FFVItemStack& Stack, const AActor* Seller) const;

	UFUNCTION(BlueprintPure, Category = "FV|Item|Trade", meta = (ToolTip = "The stock stack holds Quantity units that aren't worn, and the buyer can pay for and carry them."))
	bool CanBuy(const AActor* Buyer, FGuid StockInstanceId, int32 Quantity) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Trade", meta = (ToolTip = "Moves Quantity units of a stock stack to the buyer, who pays for them. False if Can Buy fails or not every unit moved (the units that moved are paid)."))
	bool Buy(AActor* Buyer, FGuid StockInstanceId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Trade", meta = (ToolTip = "Buys up to Quantity units of the item, stack by stack, e.g. a drink ordered in dialogue. Returns the units bought."))
	int32 BuyItem(AActor* Buyer, const UFVItemDefinition* Item, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Trade", meta = (ToolTip = "The seller's stack holds Quantity units that aren't worn, the merchant buys it and can pay for them."))
	bool CanSell(const AActor* Seller, FGuid InstanceId, int32 Quantity) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Trade", meta = (ToolTip = "Moves Quantity units of the seller's stack into the stock, keeping a Stolen mark, and pays for them. False if Can Sell fails or not every unit moved (the units that moved are paid)."))
	bool Sell(AActor* Seller, FGuid InstanceId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Trade", meta = (ToolTip = "Broadcasts Message.Item.TradeRequested with the customer as Opener and the merchant as Container; the trade screen opens on it."))
	void RequestTrade(AActor* Customer);

private:
	/** Share taken off buying prices and added to selling prices, within plus or minus 50%. */
	float GetPriceAdjustment(const AActor* Customer) const;

	bool IsRefusedAsStolen(const FFVItemStack& Stack) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant", meta = (ToolTip = "Faction whose standing and disguise change prices. None: the owner's faction (Social fragment)."))
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant", meta = (ClampMin = 0, ToolTip = "Base price multiplier when the customer buys."))
	float BuyMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant", meta = (ClampMin = 0, ToolTip = "Base price multiplier when the customer sells."))
	float SellMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant", meta = (ClampMin = 0, ToolTip = "Price change per point of the customer's standing with the faction: positive standing lowers buying prices and raises selling prices, negative does the opposite."))
	float StandingPriceFactor = 0.005f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant", meta = (ClampMin = 0, ClampMax = 0.5, ToolTip = "Extra price change for a customer disguised as one of the faction."))
	float DisguiseDiscount = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant", meta = (ToolTip = "Buys goods stolen from its own faction or from itself, which other merchants refuse."))
	bool bFence = false;
};
