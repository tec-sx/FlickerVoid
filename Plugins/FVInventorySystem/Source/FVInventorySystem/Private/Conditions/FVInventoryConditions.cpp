#include "Conditions/FVInventoryConditions.h"

#include "Equipment/FVEquipmentComponent.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDefinition.h"
#include "Loot/FVLootLibrary.h"
#include "Loot/FVLootTableDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryConditions)

#define LOCTEXT_NAMESPACE "FVInventoryConditions"

namespace FVInventoryConditions
{
	FText ItemName(const UFVItemDefinition* Item)
	{
		return Item ? Item->Display.Name : LOCTEXT("None", "<none>");
	}
}

bool FFVCondition_HasItem::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bCheckTarget, StaticStruct()));
	return Inventory && Inventory->HasItem(Item, Quantity);
}

FText FFVCondition_HasItem::GetDescription() const
{
	return FText::Format(LOCTEXT("HasItem", "Has {0} x{1}"), FVInventoryConditions::ItemName(Item), Quantity);
}

bool FFVCondition_HasItemWithTag::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bCheckTarget, StaticStruct()));
	return Inventory && Tag.IsValid() && Inventory->GetQuantityWithTag(Tag) >= Quantity;
}

FText FFVCondition_HasItemWithTag::GetDescription() const
{
	return FText::Format(LOCTEXT("HasItemWithTag", "Has item tagged {0} x{1}"), FText::FromName(Tag.GetTagName()), Quantity);
}

bool FFVCondition_HasMoney::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bCheckTarget, StaticStruct()));
	return Inventory && Inventory->CanAfford(Amount);
}

FText FFVCondition_HasMoney::GetDescription() const
{
	return FText::Format(LOCTEXT("HasMoney", "Has {0} money"), Amount);
}

bool FFVCondition_IsWearing::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(Context.GetActor(bCheckTarget, StaticStruct()));
	if (!Equipment)
	{
		return false;
	}
	return Item ? Equipment->IsEquipped(Item) : Equipment->GetGrantedTags().HasTag(GrantedTag);
}

FText FFVCondition_IsWearing::GetDescription() const
{
	return Item
		? FText::Format(LOCTEXT("WearingItem", "Wearing {0}"), Item->Display.Name)
		: FText::Format(LOCTEXT("WearingTag", "Wearing {0}"), FText::FromName(GrantedTag.GetTagName()));
}

void FFVEffect_GiveItem::Apply(const FFVConditionContext& Context) const
{
	AActor* Receiver = Context.GetActor(bGiveToTarget, StaticStruct());
	UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Receiver);
	// Story grants ignore carry limits so a reward is never lost; the player drops something later.
	if (!Inventory || Inventory->AddStoryItem(Item, Quantity) <= 0 || !bEquip)
	{
		return;
	}

	if (UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(Receiver))
	{
		Equipment->Equip(Item);
	}
}

FText FFVEffect_GiveItem::GetDescription() const
{
	return FText::Format(LOCTEXT("GiveItem", "Give {0} x{1}"), FVInventoryConditions::ItemName(Item), Quantity);
}

void FFVEffect_TakeItem::Apply(const FFVConditionContext& Context) const
{
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bTakeFromTarget, StaticStruct())))
	{
		Inventory->RemoveItem(Item, Quantity);
	}
}

FText FFVEffect_TakeItem::GetDescription() const
{
	return FText::Format(LOCTEXT("TakeItem", "Take {0} x{1}"), FVInventoryConditions::ItemName(Item), Quantity);
}

void FFVEffect_GiveMoney::Apply(const FFVConditionContext& Context) const
{
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bGiveToTarget, StaticStruct())))
	{
		Inventory->AddMoney(Amount);
	}
}

FText FFVEffect_GiveMoney::GetDescription() const
{
	return FText::Format(LOCTEXT("GiveMoney", "Give {0} money"), Amount);
}

void FFVEffect_TakeMoney::Apply(const FFVConditionContext& Context) const
{
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bTakeFromTarget, StaticStruct())))
	{
		Inventory->RemoveMoney(Amount);
	}
}

FText FFVEffect_TakeMoney::GetDescription() const
{
	return FText::Format(LOCTEXT("TakeMoney", "Take {0} money"), Amount);
}

void FFVEffect_GiveLoot::Apply(const FFVConditionContext& Context) const
{
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Context.GetActor(bGiveToTarget, StaticStruct())))
	{
		UFVLootLibrary::GiveLoot(Table, Inventory, Context.Instigator);
	}
}

FText FFVEffect_GiveLoot::GetDescription() const
{
	return FText::Format(LOCTEXT("GiveLoot", "Give loot from {0}"), Table ? FText::FromName(Table->GetFName()) : LOCTEXT("NoTable", "<none>"));
}

#undef LOCTEXT_NAMESPACE
