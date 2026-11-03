#include "Items/FVMerchantComponent.h"

#include "Data/FVCharacterDefinition.h"
#include "Equipment/FVEquipmentComponent.h"
#include "FVGameplayTags.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Identity/FVIdentityComponent.h"
#include "Inventory/FVItemStackLibrary.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemMessages.h"
#include "Scales/FVScaleLibrary.h"
#include "Social/FVFactionDefinition.h"
#include "Social/FVSocialLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMerchantComponent)

namespace FVMerchant
{
	bool IsWorn(const AActor* Wearer, const FGuid& InstanceId)
	{
		const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(Wearer);
		return Equipment && Equipment->IsInstanceEquipped(InstanceId);
	}
}

UFVInventoryComponent* UFVMerchantComponent::GetStock() const
{
	return UFVInventoryComponent::Find(GetOwner());
}

UFVFactionDefinition* UFVMerchantComponent::GetPriceFaction() const
{
	return Faction ? Faction.Get() : UFVSocialLibrary::GetActorFaction(GetOwner());
}

int32 UFVMerchantComponent::GetBuyPrice(const UFVItemDefinition* Item, const AActor* Buyer) const
{
	const int32 BasePrice = UFVItemStackLibrary::GetBasePrice(Item);
	if (BasePrice <= 0)
	{
		return 0;
	}

	return FMath::Max(1, FMath::RoundToInt32(BasePrice * BuyMultiplier * (1.f - GetPriceAdjustment(Buyer))));
}

int32 UFVMerchantComponent::GetSellPrice(const FFVItemStack& Stack, const AActor* Seller) const
{
	if (!Stack.Item || Stack.Item->bQuestItem || !UFVItemStackLibrary::IsSellable(Stack.Item) || IsRefusedAsStolen(Stack))
	{
		return 0;
	}

	return FMath::RoundToInt32(UFVItemStackLibrary::GetBasePrice(Stack.Item) * SellMultiplier * (1.f + GetPriceAdjustment(Seller)));
}

bool UFVMerchantComponent::CanBuy(const AActor* Buyer, const FGuid StockInstanceId, const int32 Quantity) const
{
	const UFVInventoryComponent* StockInventory = GetStock();
	const UFVInventoryComponent* BuyerInventory = Buyer && Buyer != GetOwner() ? UFVInventoryComponent::Find(Buyer) : nullptr;
	const FFVItemStack* Stack = StockInventory ? StockInventory->FindStack(StockInstanceId) : nullptr;

	if (!BuyerInventory || Quantity <= 0 || !Stack || Stack->Quantity < Quantity || FVMerchant::IsWorn(GetOwner(), StockInstanceId))
	{
		return false;
	}

	return BuyerInventory->CanAfford(GetBuyPrice(Stack->Item, Buyer) * Quantity) && BuyerInventory->CanAddItem(Stack->Item, Quantity);
}

bool UFVMerchantComponent::Buy(AActor* Buyer, const FGuid StockInstanceId, const int32 Quantity)
{
	if (!CanBuy(Buyer, StockInstanceId, Quantity))
	{
		return false;
	}

	UFVInventoryComponent* StockInventory = GetStock();
	UFVInventoryComponent* BuyerInventory = UFVInventoryComponent::Find(Buyer);
	const int32 UnitPrice = GetBuyPrice(StockInventory->FindStack(StockInstanceId)->Item, Buyer);
	const int32 Bought = StockInventory->TransferStack(BuyerInventory, StockInstanceId, Quantity);

	if (Bought > 0)
	{
		BuyerInventory->TransferMoney(StockInventory, UnitPrice * Bought);
	}

	return Bought == Quantity;
}

int32 UFVMerchantComponent::BuyItem(AActor* Buyer, const UFVItemDefinition* Item, const int32 Quantity)
{
	const UFVInventoryComponent* StockInventory = GetStock();
	if (!StockInventory || !Item)
	{
		return 0;
	}

	int32 Bought = 0;

	// A copy, since buying changes the stock.
	const TArray<FFVItemStack> Stacks = StockInventory->GetItems();
	for (const FFVItemStack& Stack : Stacks)
	{
		if (Bought >= Quantity)
		{
			break;
		}

		if (Stack.Item != Item)
		{
			continue;
		}

		const int32 Wanted = FMath::Min(Quantity - Bought, Stack.Quantity);
		if (Buy(Buyer, Stack.InstanceId, Wanted))
		{
			Bought += Wanted;
		}
	}

	return Bought;
}

bool UFVMerchantComponent::CanSell(const AActor* Seller, const FGuid InstanceId, const int32 Quantity) const
{
	const UFVInventoryComponent* StockInventory = GetStock();
	const UFVInventoryComponent* SellerInventory = Seller && Seller != GetOwner() ? UFVInventoryComponent::Find(Seller) : nullptr;
	const FFVItemStack* Stack = SellerInventory ? SellerInventory->FindStack(InstanceId) : nullptr;

	if (!StockInventory || Quantity <= 0 || !Stack || Stack->Quantity < Quantity || FVMerchant::IsWorn(Seller, InstanceId))
	{
		return false;
	}

	const int32 UnitPrice = GetSellPrice(*Stack, Seller);
	return UnitPrice > 0 && StockInventory->CanAfford(UnitPrice * Quantity);
}

bool UFVMerchantComponent::Sell(AActor* Seller, const FGuid InstanceId, const int32 Quantity)
{
	if (!CanSell(Seller, InstanceId, Quantity))
	{
		return false;
	}

	UFVInventoryComponent* StockInventory = GetStock();
	UFVInventoryComponent* SellerInventory = UFVInventoryComponent::Find(Seller);
	const int32 UnitPrice = GetSellPrice(*SellerInventory->FindStack(InstanceId), Seller);
	const int32 Sold = SellerInventory->TransferStack(StockInventory, InstanceId, Quantity);

	if (Sold > 0)
	{
		StockInventory->TransferMoney(SellerInventory, UnitPrice * Sold);
	}

	return Sold == Quantity;
}

void UFVMerchantComponent::RequestTrade(AActor* Customer)
{
	FFVItemContainerMessage Message;
	Message.Opener = Customer;
	Message.Container = GetOwner();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVGameplayTags::Message_Item_TradeRequested, Message);
}

float UFVMerchantComponent::GetPriceAdjustment(const AActor* Customer) const
{
	const UFVFactionDefinition* MerchantFaction = GetPriceFaction();
	if (!MerchantFaction)
	{
		return 0.f;
	}

	const int32 Standing = UFVScaleLibrary::GetValue(this, MerchantFaction->Standing);
	float Adjustment = Standing * StandingPriceFactor;
	if (Customer && UFVSocialLibrary::IsDisguisedAs(Customer, MerchantFaction))
	{
		Adjustment += DisguiseDiscount;
	}

	return FMath::Clamp(Adjustment, -0.5f, 0.5f);
}

bool UFVMerchantComponent::IsRefusedAsStolen(const FFVItemStack& Stack) const
{
	FGameplayTag StolenFrom;
	if (bFence || !UFVItemStackLibrary::IsStolen(Stack, StolenFrom) || !StolenFrom.IsValid())
	{
		return false;
	}

	const UFVFactionDefinition* MerchantFaction = GetPriceFaction();
	if (MerchantFaction && StolenFrom == MerchantFaction->Id)
	{
		return true;
	}

	const UFVCharacterDefinition* Character = UFVIdentityComponent::GetActorDefinition(GetOwner());
	return Character && StolenFrom == Character->Id;
}
