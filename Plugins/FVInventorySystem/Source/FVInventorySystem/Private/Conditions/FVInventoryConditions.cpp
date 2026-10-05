#include "Conditions/FVInventoryConditions.h"

#include "Equipment/FVEquipmentComponent.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryConditions)

#define LOCTEXT_NAMESPACE "FVInventoryConditions"

namespace FVInventoryConditions
{
	AActor* Pick(const FFVConditionContext& Context, const bool bTarget)
	{
		return bTarget ? Context.Target.Get() : Context.Instigator.Get();
	}

	FText ItemName(const UFVItemDefinition* Item)
	{
		return Item ? Item->Display.Name : LOCTEXT("None", "<none>");
	}
}

bool FFVCondition_HasItem::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(FVInventoryConditions::Pick(Context, bCheckTarget));
	return Inventory && Inventory->HasItem(Item, Quantity);
}

FText FFVCondition_HasItem::GetDescription() const
{
	return FText::Format(LOCTEXT("HasItem", "Has {0} x{1}"), FVInventoryConditions::ItemName(Item), Quantity);
}

bool FFVCondition_IsWearing::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(FVInventoryConditions::Pick(Context, bCheckTarget));
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
	AActor* Receiver = FVInventoryConditions::Pick(Context, bGiveToTarget);
	UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Receiver);
	if (!Inventory || Inventory->AddItem(Item, Quantity) <= 0 || !bEquip)
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
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(FVInventoryConditions::Pick(Context, bTakeFromTarget)))
	{
		Inventory->RemoveItem(Item, Quantity);
	}
}

FText FFVEffect_TakeItem::GetDescription() const
{
	return FText::Format(LOCTEXT("TakeItem", "Take {0} x{1}"), FVInventoryConditions::ItemName(Item), Quantity);
}

#undef LOCTEXT_NAMESPACE
