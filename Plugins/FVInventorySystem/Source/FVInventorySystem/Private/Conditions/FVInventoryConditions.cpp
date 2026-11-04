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
	AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor);
	return Inventory && Inventory->HasItem(Item, Quantity);
}

FText FFVCondition_HasItem::GetDescription() const
{
	return FText::Format(LOCTEXT("HasItem", "Has {0} x{1}"), FVInventoryConditions::ItemName(Item), Quantity);
}

bool FFVCondition_HasItemWithTag::EvaluateImpl(const FFVConditionContext& Context) const
{
	AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor);
	return Inventory && Tag.IsValid() && Inventory->GetQuantityWithTag(Tag) >= Quantity;
}

FText FFVCondition_HasItemWithTag::GetDescription() const
{
	return FText::Format(LOCTEXT("HasItemWithTag", "Has item tagged {0} x{1}"), FText::FromName(Tag.GetTagName()), Quantity);
}

bool FFVCondition_HasMoney::EvaluateImpl(const FFVConditionContext& Context) const
{
	AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor);
	return Inventory && Inventory->CanAfford(Amount);
}

FText FFVCondition_HasMoney::GetDescription() const
{
	return FText::Format(LOCTEXT("HasMoney", "Has {0} money"), Amount);
}

bool FFVCondition_IsWearing::EvaluateImpl(const FFVConditionContext& Context) const
{
	AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(Actor);
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
	AActor* Actor = Context.GetActor(bTakeFromTarget, StaticStruct());
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor))
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
	AActor* Actor = Context.GetActor(bGiveToTarget, StaticStruct());
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor))
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
	AActor* Actor = Context.GetActor(bTakeFromTarget, StaticStruct());
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor))
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
	AActor* Actor = Context.GetActor(bGiveToTarget, StaticStruct());
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Actor))
	{
		UFVLootLibrary::GiveLoot(Table, Inventory, Context.Instigator);
	}
}

FText FFVEffect_GiveLoot::GetDescription() const
{
	return FText::Format(LOCTEXT("GiveLoot", "Give loot from {0}"), Table ? FText::FromName(Table->GetFName()) : LOCTEXT("NoTable", "<none>"));
}

FText FFVCondition_HasItem::GetSetupError() const
{
	return Item ? FText::GetEmpty() : LOCTEXT("NoItem", "no item");
}

bool FFVCondition_HasItem::CanEvaluate(const FFVConditionContext& Context) const
{
	return Item && Context.GetActor(bCheckTarget, StaticStruct());
}

FText FFVCondition_HasItemWithTag::GetSetupError() const
{
	return Tag.IsValid() ? FText::GetEmpty() : LOCTEXT("NoTag", "no tag");
}

bool FFVCondition_HasItemWithTag::CanEvaluate(const FFVConditionContext& Context) const
{
	return Tag.IsValid() && Context.GetActor(bCheckTarget, StaticStruct());
}

bool FFVCondition_HasMoney::CanEvaluate(const FFVConditionContext& Context) const
{
	return Context.GetActor(bCheckTarget, StaticStruct()) != nullptr;
}

FText FFVCondition_IsWearing::GetSetupError() const
{
	return Item || GrantedTag.IsValid() ? FText::GetEmpty() : LOCTEXT("NoWorn", "no item or granted tag");
}

bool FFVCondition_IsWearing::CanEvaluate(const FFVConditionContext& Context) const
{
	return (Item || GrantedTag.IsValid()) && Context.GetActor(bCheckTarget, StaticStruct());
}

FText FFVEffect_GiveItem::GetSetupError() const
{
	return Item ? FText::GetEmpty() : LOCTEXT("NoItem", "no item");
}

FText FFVEffect_TakeItem::GetSetupError() const
{
	return Item ? FText::GetEmpty() : LOCTEXT("NoItem", "no item");
}

FText FFVEffect_GiveLoot::GetSetupError() const
{
	return Table ? FText::GetEmpty() : LOCTEXT("NoTable", "no loot table");
}

#undef LOCTEXT_NAMESPACE
