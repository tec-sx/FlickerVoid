#include "Equipment/FVEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Conditions/FVConditionStatics.h"
#include "GameFramework/Actor.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"
#include "Save/FVSaveableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVEquipmentComponent)

UFVEquipmentComponent::UFVEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVEquipmentComponent* UFVEquipmentComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVEquipmentComponent>() : nullptr;
}

void UFVEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner()))
	{
		Inventory->OnItemChanged.AddUniqueDynamic(this, &UFVEquipmentComponent::HandleInventoryChanged);
	}

	if (UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(GetOwner()))
	{
		Saveable->OnActorDataLoaded.AddUniqueDynamic(this, &UFVEquipmentComponent::HandleActorDataLoaded);
	}

	RefreshGrantedTags();
}

void UFVEquipmentComponent::HandleActorDataLoaded()
{
	RefreshGrantedTags();

	// Listeners (meshes, inventory capacity) rebuild from the restored slots.
	for (const FFVEquippedItem& Entry : Equipped)
	{
		OnEquipmentChanged.Broadcast(Entry.Slot, nullptr, Entry.Item);
	}
}

void UFVEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner()))
	{
		Inventory->OnItemChanged.RemoveDynamic(this, &UFVEquipmentComponent::HandleInventoryChanged);
	}

	RemoveAppliedTags();
	Super::EndPlay(EndPlayReason);
}

bool UFVEquipmentComponent::CanEquip(const UFVItemDefinition* Item) const
{
	const FFVItemFragment_Equippable* Equippable = Item ? Item->FindFragment<FFVItemFragment_Equippable>() : nullptr;
	if (!Equippable || !Equippable->Slot.IsValid())
	{
		return false;
	}

	if (bRequireInventory)
	{
		const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
		if (!Inventory || !Inventory->HasItem(Item))
		{
			return false;
		}
	}

	return Equippable->EquipConditions.Evaluate(UFVConditionStatics::MakeContext(GetOwner(), GetOwner()));
}

bool UFVEquipmentComponent::Equip(UFVItemDefinition* Item)
{
	if (!CanEquip(Item) || IsEquipped(Item))
	{
		return false;
	}

	SetSlot(Item->GetEquipmentSlot(), Item);
	return true;
}

bool UFVEquipmentComponent::Unequip(const FGameplayTag Slot)
{
	if (!GetEquipped(Slot))
	{
		return false;
	}

	SetSlot(Slot, nullptr);
	return true;
}

UFVItemDefinition* UFVEquipmentComponent::GetEquipped(const FGameplayTag Slot) const
{
	const FFVEquippedItem* Entry = Equipped.FindByPredicate([&Slot](const FFVEquippedItem& It) { return It.Slot == Slot; });
	return Entry ? Entry->Item.Get() : nullptr;
}

bool UFVEquipmentComponent::IsEquipped(const UFVItemDefinition* Item) const
{
	return Item && Equipped.ContainsByPredicate([Item](const FFVEquippedItem& It) { return It.Item == Item; });
}

FGameplayTagContainer UFVEquipmentComponent::GetGrantedTags() const
{
	FGameplayTagContainer Tags;
	for (const FFVEquippedItem& Entry : Equipped)
	{
		const FFVItemFragment_Equippable* Equippable = Entry.Item ? Entry.Item->FindFragment<FFVItemFragment_Equippable>() : nullptr;
		if (Equippable)
		{
			Tags.AppendTags(Equippable->GrantedTags);
		}
	}
	return Tags;
}

void UFVEquipmentComponent::RefreshGrantedTags()
{
	RemoveAppliedTags();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC)
	{
		return;
	}

	AppliedTags = GetGrantedTags();
	ASC->AddLooseGameplayTags(AppliedTags);
	AppliedTo = ASC;
}

void UFVEquipmentComponent::RemoveAppliedTags()
{
	if (UAbilitySystemComponent* ASC = AppliedTo.Get())
	{
		ASC->RemoveLooseGameplayTags(AppliedTags);
	}
	AppliedTo.Reset();
	AppliedTags.Reset();
}

void UFVEquipmentComponent::HandleInventoryChanged(UFVItemDefinition* Item, const int32 OldQuantity, const int32 NewQuantity)
{
	if (bRequireInventory && NewQuantity <= 0 && IsEquipped(Item))
	{
		Unequip(Item->GetEquipmentSlot());
	}
}

void UFVEquipmentComponent::SetSlot(const FGameplayTag Slot, UFVItemDefinition* Item)
{
	UFVItemDefinition* OldItem = GetEquipped(Slot);

	Equipped.RemoveAll([&Slot](const FFVEquippedItem& It) { return It.Slot == Slot; });
	if (Item)
	{
		FFVEquippedItem& Entry = Equipped.AddDefaulted_GetRef();
		Entry.Slot = Slot;
		Entry.Item = Item;
	}

	RefreshGrantedTags();
	OnEquipmentChanged.Broadcast(Slot, OldItem, Item);
}
