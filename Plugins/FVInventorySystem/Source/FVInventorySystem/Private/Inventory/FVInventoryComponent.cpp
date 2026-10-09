#include "Inventory/FVInventoryComponent.h"

#include "Conditions/FVConditionLibrary.h"
#include "Equipment/FVEquipmentComponent.h"
#include "FVInventorySettings.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/Actor.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"
#include "Pickup/FVItemReceiverComponent.h"
#include "Save/FVSaveableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryComponent)

namespace
{
	TAutoConsoleVariable<int32> CVarEnforceWeight(
		TEXT("FVCvar.Inventory.EnforceWeight"),
		-1,
		TEXT("Override the weight limit setting. -1 use settings, 0 off, 1 on."));

	TAutoConsoleVariable<int32> CVarEnforceSpace(
		TEXT("FVCvar.Inventory.EnforceSpace"),
		-1,
		TEXT("Override the grid space setting. -1 use settings, 0 off, 1 on."));

	bool IsEnforced(const TAutoConsoleVariable<int32>& CVar, const bool bSetting)
	{
		const int32 Override = CVar.GetValueOnGameThread();
		return Override < 0 ? bSetting : Override > 0;
	}
}

UFVInventoryComponent::UFVInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVInventoryComponent* UFVInventoryComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVInventoryComponent>() : nullptr;
}

void UFVInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->OnItemReceived.AddUniqueDynamic(this, &UFVInventoryComponent::HandleItemReceived);
	}

	if (UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner()))
	{
		Equipment->OnEquipmentChanged.AddUniqueDynamic(this, &UFVInventoryComponent::HandleEquipmentChanged);
	}

	if (UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(GetOwner()))
	{
		Saveable->OnActorDataLoaded.AddUniqueDynamic(this, &UFVInventoryComponent::HandleActorDataLoaded);
	}

	RefreshContainers();
}

void UFVInventoryComponent::HandleActorDataLoaded()
{
	RefreshContainers();
	OnCapacityChanged.Broadcast();

	for (const FFVItemStack& Stack : Items)
	{
		OnItemChanged.Broadcast(Stack.Item, 0, Stack.Quantity);
	}
}

void UFVInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->OnItemReceived.RemoveDynamic(this, &UFVInventoryComponent::HandleItemReceived);
	}

	if (UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner()))
	{
		Equipment->OnEquipmentChanged.RemoveDynamic(this, &UFVInventoryComponent::HandleEquipmentChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInventoryComponent::HandleItemReceived(UFVItemDefinition* Item, const int32 Quantity, AActor* Source)
{
	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->ReportTaken(AddItem(Item, Quantity));
	}
}

void UFVInventoryComponent::HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem)
{
	RefreshContainers();
}

void UFVInventoryComponent::RefreshContainers()
{
	const float OldWeight = ContainerWeightBonus;
	const int32 OldCells = ContainerCells;

	ContainerWeightBonus = 0.f;
	ContainerCells = 0;

	if (const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner()))
	{
		for (const FFVEquippedItem& Entry : Equipment->GetAllEquipped())
		{
			if (Entry.Item == nullptr)
			{
				continue;
			}

			if (const FFVItemFragment_Container* Container = Entry.Item->FindFragment<FFVItemFragment_Container>())
			{
				ContainerWeightBonus += Container->WeightBonus;
				ContainerCells += Container->Cells;
			}
		}
	}

	if (!FMath::IsNearlyEqual(OldWeight, ContainerWeightBonus) || OldCells != ContainerCells)
	{
		OnCapacityChanged.Broadcast();
	}
}

float UFVInventoryComponent::GetTotalWeight() const
{
	float Total = 0.f;
	for (const FFVItemStack& Stack : Items)
	{
		if (Stack.Item != nullptr)
		{
			Total += Stack.Item->Weight * Stack.Quantity;
		}
	}
	return Total;
}

float UFVInventoryComponent::GetWeightLimit() const
{
	return UFVInventorySettings::Get().BaseWeightLimit + ContainerWeightBonus + WeightBonus;
}

int32 UFVInventoryComponent::GetUsedCells() const
{
	int32 Used = 0;
	for (const FFVItemStack& Stack : Items)
	{
		if (Stack.Item != nullptr)
		{
			Used += Stack.Item->GetCellCount();
		}
	}
	return Used;
}

int32 UFVInventoryComponent::GetCellCapacity() const
{
	return UFVInventorySettings::Get().BasePocketCells + ContainerCells + CellBonus;
}

void UFVInventoryComponent::SetBonuses(const float InWeightBonus, const int32 InCellBonus)
{
	if (FMath::IsNearlyEqual(WeightBonus, InWeightBonus) && CellBonus == InCellBonus)
	{
		return;
	}

	WeightBonus = InWeightBonus;
	CellBonus = InCellBonus;
	OnCapacityChanged.Broadcast();
}

int32 UFVInventoryComponent::GetAcceptedQuantity(const UFVItemDefinition* Item, const int32 Quantity) const
{
	if (Item == nullptr || Quantity <= 0)
	{
		return 0;
	}

	const UFVInventorySettings& Settings = UFVInventorySettings::Get();
	const int32 Current = GetQuantity(Item);

	int32 Accepted = Item->MaxQuantity > 0 ? FMath::Clamp(Item->MaxQuantity - Current, 0, Quantity) : Quantity;

	if (Accepted > 0 && IsEnforced(CVarEnforceWeight, Settings.bEnforceWeight) && Item->Weight > 0.f)
	{
		const float Room = GetWeightLimit() - GetTotalWeight();
		Accepted = FMath::Clamp(FMath::FloorToInt32(Room / Item->Weight), 0, Accepted);
	}

	// A new stack claims its footprint once; adding to an existing stack claims nothing more.
	if (Accepted > 0 && Current == 0 && IsEnforced(CVarEnforceSpace, Settings.bEnforceSpace))
	{
		if (GetUsedCells() + Item->GetCellCount() > GetCellCapacity())
		{
			Accepted = 0;
		}
	}

	return Accepted;
}

int32 UFVInventoryComponent::AddItem(UFVItemDefinition* Item, const int32 Quantity)
{
	const int32 Added = GetAcceptedQuantity(Item, Quantity);
	if (Added > 0)
	{
		SetQuantity(Item, GetQuantity(Item) + Added);
	}
	return Added;
}

bool UFVInventoryComponent::RemoveItem(UFVItemDefinition* Item, const int32 Quantity)
{
	const int32 Current = GetQuantity(Item);
	if (!Item || Quantity <= 0 || Current < Quantity)
	{
		return false;
	}

	SetQuantity(Item, Current - Quantity);
	return true;
}

int32 UFVInventoryComponent::GetQuantity(const UFVItemDefinition* Item) const
{
	const FFVItemStack* Stack = Items.FindByPredicate([Item](const FFVItemStack& Entry) { return Entry.Item == Item; });
	return Stack ? Stack->Quantity : 0;
}

TArray<FFVItemStack> UFVInventoryComponent::GetItemsInCategory(const FGameplayTag Category) const
{
	return Items.FilterByPredicate([&Category](const FFVItemStack& Entry)
	{
		return Entry.Item && Entry.Item->Category.MatchesTag(Category);
	});
}

bool UFVInventoryComponent::CanUseItem(const UFVItemDefinition* Item) const
{
	const FFVItemFragment_Usable* Usable = Item ? Item->FindFragment<FFVItemFragment_Usable>() : nullptr;
	if (!Usable || !HasItem(Item))
	{
		return false;
	}
	return Usable->UseConditions.Evaluate(UFVConditionLibrary::MakeContext(GetOwner(), GetOwner()));
}

bool UFVInventoryComponent::UseItem(UFVItemDefinition* Item)
{
	if (!CanUseItem(Item))
	{
		return false;
	}

	const FFVItemFragment_Usable* Usable = Item->FindFragment<FFVItemFragment_Usable>();
	Usable->Effects.Apply(UFVConditionLibrary::MakeContext(GetOwner(), GetOwner()));
	if (Usable->bConsumeOnUse)
	{
		RemoveItem(Item, 1);
	}

	OnItemUsed.Broadcast(Item);
	return true;
}

FFVItemStack* UFVInventoryComponent::FindStack(const UFVItemDefinition* Item)
{
	return Items.FindByPredicate([Item](const FFVItemStack& Entry) { return Entry.Item == Item; });
}

void UFVInventoryComponent::SetQuantity(UFVItemDefinition* Item, const int32 NewQuantity)
{
	FFVItemStack* Stack = FindStack(Item);
	const int32 OldQuantity = Stack ? Stack->Quantity : 0;
	if (OldQuantity == NewQuantity)
	{
		return;
	}

	if (NewQuantity <= 0)
	{
		Items.RemoveAll([Item](const FFVItemStack& Entry) { return Entry.Item == Item; });
	}
	else if (Stack)
	{
		Stack->Quantity = NewQuantity;
	}
	else
	{
		FFVItemStack& NewStack = Items.AddDefaulted_GetRef();
		NewStack.Item = Item;
		NewStack.Quantity = NewQuantity;
	}

	OnItemChanged.Broadcast(Item, OldQuantity, FMath::Max(NewQuantity, 0));
	OnCapacityChanged.Broadcast();
}
