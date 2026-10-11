#include "Equipment/FVEquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Conditions/FVConditionLibrary.h"
#include "GameFramework/Actor.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"
#include "Save/FVSaveableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVEquipmentComponent)

namespace
{
	const FFVItemFragment_Equippable* GetEquippable(const UFVItemDefinition* Item)
	{
		return Item ? Item->FindFragment<FFVItemFragment_Equippable>() : nullptr;
	}

	bool Conflicts(const FFVEquippedItem& Entry, const FGameplayTag& Slot, const FFVItemFragment_Equippable& NewItem)
	{
		if (Entry.Slot == Slot || NewItem.BlockedSlots.HasTagExact(Entry.Slot))
		{
			return true;
		}
		const FFVItemFragment_Equippable* Worn = GetEquippable(Entry.Item);
		return Worn && Worn->BlockedSlots.HasTagExact(Slot);
	}
}

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
		Inventory->OnStackChanged.AddUniqueDynamic(this, &UFVEquipmentComponent::HandleStackChanged);
	}

	if (UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(GetOwner()))
	{
		Saveable->OnActorDataLoaded.AddUniqueDynamic(this, &UFVEquipmentComponent::HandleActorDataLoaded);
	}

	LinkStartingInstances();
	RefreshGrantedTags();
}

void UFVEquipmentComponent::HandleActorDataLoaded()
{
	LinkStartingInstances();
	RefreshGrantedTags();
	OnEquipmentReset.Broadcast();
}

void UFVEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner()))
	{
		Inventory->OnStackChanged.RemoveDynamic(this, &UFVEquipmentComponent::HandleStackChanged);
	}

	RemoveAppliedTags();
	Super::EndPlay(EndPlayReason);
}

void UFVEquipmentComponent::LinkStartingInstances()
{
	for (FFVEquippedItem& Entry : Equipped)
	{
		if (!Entry.InstanceId.IsValid() && Entry.Item)
		{
			ResolveInstance(Entry.Item, Entry.InstanceId);
		}
	}
}

bool UFVEquipmentComponent::ResolveInstance(const UFVItemDefinition* Item, FGuid& OutInstanceId) const
{
	OutInstanceId.Invalidate();
	if (const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner()))
	{
		for (const FFVItemStack& Stack : Inventory->GetItems())
		{
			if (Stack.Item == Item && !IsInstanceEquipped(Stack.InstanceId))
			{
				OutInstanceId = Stack.InstanceId;
				return true;
			}
		}
	}
	return !bRequireInventory;
}

bool UFVEquipmentComponent::CanEquip(const UFVItemDefinition* Item) const
{
	FGuid InstanceId;
	FGameplayTag Slot;
	TArray<FGameplayTag> Removed;
	return ResolveInstance(Item, InstanceId) && PlanEquip(Item, InstanceId, FGameplayTag(), Slot, Removed);
}

bool UFVEquipmentComponent::Equip(UFVItemDefinition* Item)
{
	FGuid InstanceId;
	FGameplayTag Slot;
	TArray<FGameplayTag> Removed;
	if (!ResolveInstance(Item, InstanceId) || !PlanEquip(Item, InstanceId, FGameplayTag(), Slot, Removed))
	{
		return false;
	}

	Apply(Slot, Item, InstanceId, Removed);
	return true;
}

bool UFVEquipmentComponent::CanEquipInstance(const FGuid InstanceId, const FGameplayTag Slot) const
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
	const FFVItemStack* Stack = Inventory ? Inventory->FindStack(InstanceId) : nullptr;
	FGameplayTag ChosenSlot;
	TArray<FGameplayTag> Removed;
	return Stack && PlanEquip(Stack->Item, InstanceId, Slot, ChosenSlot, Removed);
}

bool UFVEquipmentComponent::EquipInstance(const FGuid InstanceId, const FGameplayTag Slot)
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
	const FFVItemStack* Stack = Inventory ? Inventory->FindStack(InstanceId) : nullptr;
	FGameplayTag ChosenSlot;
	TArray<FGameplayTag> Removed;
	if (!Stack || !PlanEquip(Stack->Item, InstanceId, Slot, ChosenSlot, Removed))
	{
		return false;
	}

	Apply(ChosenSlot, Stack->Item, InstanceId, Removed);
	return true;
}

bool UFVEquipmentComponent::PlanEquip(const UFVItemDefinition* Item, const FGuid& InstanceId, const FGameplayTag RequestedSlot, FGameplayTag& OutSlot, TArray<FGameplayTag>& OutRemoved) const
{
	const FFVItemFragment_Equippable* Equippable = GetEquippable(Item);
	if (!Equippable || Equippable->Slots.IsEmpty())
	{
		return false;
	}

	// Without a stack to tell copies apart, one worn copy per item. A worn stack moves only to a named slot.
	if ((!InstanceId.IsValid() && IsEquipped(Item)) || (!RequestedSlot.IsValid() && IsInstanceEquipped(InstanceId)))
	{
		return false;
	}

	TArray<FGameplayTag> Candidates;
	if (RequestedSlot.IsValid())
	{
		if (!Equippable->Slots.HasTagExact(RequestedSlot))
		{
			return false;
		}
		Candidates.Add(RequestedSlot);
	}
	else
	{
		Equippable->Slots.GetGameplayTagArray(Candidates);
	}

	auto CollectConflicts = [this, Equippable, &InstanceId](const FGameplayTag& Slot)
	{
		TArray<FGameplayTag> Result;
		for (const FFVEquippedItem& Entry : Equipped)
		{
			// Moving a worn stack to another slot takes it off its current one.
			if (Conflicts(Entry, Slot, *Equippable) || (InstanceId.IsValid() && Entry.InstanceId == InstanceId))
			{
				Result.Add(Entry.Slot);
			}
		}
		return Result;
	};

	OutSlot = Candidates[0];
	OutRemoved = CollectConflicts(OutSlot);
	for (const FGameplayTag& Candidate : Candidates)
	{
		TArray<FGameplayTag> Conflicting = CollectConflicts(Candidate);
		if (Conflicting.IsEmpty())
		{
			OutSlot = Candidate;
			OutRemoved.Reset();
			break;
		}
	}

	if (const FFVEquippedItem* Current = FindEntry(OutSlot); Current && Current->Item == Item && Current->InstanceId == InstanceId)
	{
		return false;
	}

	return FitsCapacity(OutRemoved, Item)
		&& Equippable->EquipConditions.Evaluate(UFVConditionLibrary::MakeContext(GetOwner(), GetOwner()));
}

bool UFVEquipmentComponent::FitsCapacity(const TArray<FGameplayTag>& Removed, const UFVItemDefinition* Added) const
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
	if (!Inventory)
	{
		return true;
	}

	float WeightLimitChange = 0.f;
	int32 CellChange = 0;
	auto Accumulate = [&WeightLimitChange, &CellChange](const UFVItemDefinition* Item, const int32 Sign)
	{
		if (const FFVItemFragment_Container* Container = Item ? Item->FindFragment<FFVItemFragment_Container>() : nullptr)
		{
			WeightLimitChange += Sign * Container->WeightBonus;
			CellChange += Sign * Container->Cells;
		}
	};

	for (const FGameplayTag& Slot : Removed)
	{
		Accumulate(GetEquipped(Slot), -1);
	}
	Accumulate(Added, 1);

	return Inventory->FitsAfterChange(WeightLimitChange, CellChange);
}

bool UFVEquipmentComponent::CanUnequip(const FGameplayTag Slot) const
{
	return GetEquipped(Slot) && FitsCapacity({ Slot }, nullptr);
}

bool UFVEquipmentComponent::Unequip(const FGameplayTag Slot)
{
	if (!CanUnequip(Slot))
	{
		return false;
	}

	Apply(Slot, nullptr, FGuid(), {});
	return true;
}

void UFVEquipmentComponent::Apply(const FGameplayTag Slot, UFVItemDefinition* Item, const FGuid& InstanceId, const TArray<FGameplayTag>& Removed)
{
	struct FChange
	{
		FGameplayTag Slot;
		UFVItemDefinition* OldItem;
		UFVItemDefinition* NewItem;
	};
	TArray<FChange> Changes;

	for (const FGameplayTag& RemovedSlot : Removed)
	{
		if (RemovedSlot != Slot)
		{
			Changes.Add({ RemovedSlot, GetEquipped(RemovedSlot), nullptr });
			Equipped.RemoveAll([&RemovedSlot](const FFVEquippedItem& It) { return It.Slot == RemovedSlot; });
		}
	}

	Changes.Add({ Slot, GetEquipped(Slot), Item });
	Equipped.RemoveAll([&Slot](const FFVEquippedItem& It) { return It.Slot == Slot; });
	if (Item)
	{
		FFVEquippedItem& Entry = Equipped.AddDefaulted_GetRef();
		Entry.Slot = Slot;
		Entry.Item = Item;
		Entry.InstanceId = InstanceId;
	}

	RefreshGrantedTags();
	for (const FChange& Change : Changes)
	{
		OnEquipmentChanged.Broadcast(Change.Slot, Change.OldItem, Change.NewItem);
	}
}

const FFVEquippedItem* UFVEquipmentComponent::FindEntry(const FGameplayTag& Slot) const
{
	return Equipped.FindByPredicate([&Slot](const FFVEquippedItem& It) { return It.Slot == Slot; });
}

UFVItemDefinition* UFVEquipmentComponent::GetEquipped(const FGameplayTag Slot) const
{
	const FFVEquippedItem* Entry = FindEntry(Slot);
	return Entry ? Entry->Item.Get() : nullptr;
}

FGuid UFVEquipmentComponent::GetEquippedInstance(const FGameplayTag Slot) const
{
	const FFVEquippedItem* Entry = FindEntry(Slot);
	return Entry ? Entry->InstanceId : FGuid();
}

UFVItemDefinition* UFVEquipmentComponent::GetBlockingItem(const FGameplayTag Slot) const
{
	for (const FFVEquippedItem& Entry : Equipped)
	{
		const FFVItemFragment_Equippable* Equippable = GetEquippable(Entry.Item);
		if (Equippable && Equippable->BlockedSlots.HasTagExact(Slot))
		{
			return Entry.Item;
		}
	}
	return nullptr;
}

bool UFVEquipmentComponent::IsEquipped(const UFVItemDefinition* Item) const
{
	return Item && Equipped.ContainsByPredicate([Item](const FFVEquippedItem& It) { return It.Item == Item; });
}

bool UFVEquipmentComponent::IsInstanceEquipped(const FGuid InstanceId) const
{
	return InstanceId.IsValid() && Equipped.ContainsByPredicate([&InstanceId](const FFVEquippedItem& It) { return It.InstanceId == InstanceId; });
}

FGameplayTagContainer UFVEquipmentComponent::GetGrantedTags() const
{
	FGameplayTagContainer Tags;
	for (const FFVEquippedItem& Entry : Equipped)
	{
		if (const FFVItemFragment_Equippable* Equippable = GetEquippable(Entry.Item))
		{
			Tags.AppendTags(Equippable->GrantedTags);
		}
	}
	return Tags;
}

void UFVEquipmentComponent::RefreshGrantedTags()
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (AppliedTo.Get() != ASC)
	{
		RemoveAppliedTags();
	}

	if (!ASC)
	{
		return;
	}

	// Only the difference, so listeners don't see unrelated tags flicker off and on.
	const FGameplayTagContainer NewTags = GetGrantedTags();
	FGameplayTagContainer ToRemove;
	FGameplayTagContainer ToAdd;
	for (const FGameplayTag& Tag : AppliedTags)
	{
		if (!NewTags.HasTagExact(Tag))
		{
			ToRemove.AddTag(Tag);
		}
	}
	for (const FGameplayTag& Tag : NewTags)
	{
		if (!AppliedTags.HasTagExact(Tag))
		{
			ToAdd.AddTag(Tag);
		}
	}

	ASC->RemoveLooseGameplayTags(ToRemove);
	ASC->AddLooseGameplayTags(ToAdd);
	AppliedTags = NewTags;
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

void UFVEquipmentComponent::HandleStackChanged(const FGuid InstanceId)
{
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
	if (!Inventory || Inventory->FindStack(InstanceId))
	{
		return;
	}

	FFVEquippedItem* Entry = Equipped.FindByPredicate([&InstanceId](const FFVEquippedItem& It) { return It.InstanceId == InstanceId; });
	if (!Entry)
	{
		return;
	}

	if (bRequireInventory)
	{
		Apply(Entry->Slot, nullptr, FGuid(), {});
	}
	else
	{
		Entry->InstanceId.Invalidate();
	}
}
