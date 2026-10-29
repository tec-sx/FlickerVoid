#include "Inventory/FVInventoryComponent.h"

#include "Algo/StableSort.h"
#include "Conditions/FVConditionLibrary.h"
#include "Equipment/FVEquipmentComponent.h"
#include "FVInventorySettings.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/Actor.h"
#include "Inventory/FVItemInstanceData.h"
#include "Inventory/FVItemStackLibrary.h"
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

	bool CanMerge(const FFVItemStack& Stack, const UFVItemDefinition* Item)
	{
		return Stack.Item == Item && Stack.InstanceData.IsEmpty();
	}

	bool IsKeyStack(const FFVItemStack& Stack)
	{
		return Stack.Item != nullptr && Stack.Item->IsKeyItem();
	}
}

UFVInventoryComponent::UFVInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

UFVInventoryComponent* UFVInventoryComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVInventoryComponent>() : nullptr;
}

void UFVInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
	AssignMissingIds();
}

void UFVInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->OnStackReceived.AddUniqueDynamic(this, &UFVInventoryComponent::HandleStackReceived);
	}

	if (UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner()))
	{
		Equipment->OnEquipmentChanged.AddUniqueDynamic(this, &UFVInventoryComponent::HandleEquipmentChanged);
		Equipment->OnEquipmentReset.AddUniqueDynamic(this, &UFVInventoryComponent::HandleEquipmentReset);
	}

	if (UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(GetOwner()))
	{
		Saveable->OnActorDataLoaded.AddUniqueDynamic(this, &UFVInventoryComponent::HandleActorDataLoaded);
	}

	RefreshContainers();
}

void UFVInventoryComponent::HandleActorDataLoaded()
{
	AssignMissingIds();
	RefreshContainers();
	OnInventoryReset.Broadcast();
	OnCapacityChanged.Broadcast();
	OnMoneyChanged.Broadcast(Money, Money);
}

void UFVInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->OnStackReceived.RemoveDynamic(this, &UFVInventoryComponent::HandleStackReceived);
	}

	if (UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner()))
	{
		Equipment->OnEquipmentChanged.RemoveDynamic(this, &UFVInventoryComponent::HandleEquipmentChanged);
		Equipment->OnEquipmentReset.RemoveDynamic(this, &UFVInventoryComponent::HandleEquipmentReset);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInventoryComponent::AssignMissingIds()
{
	for (FFVItemStack& Stack : Items)
	{
		if (!Stack.InstanceId.IsValid())
		{
			Stack.InstanceId = FGuid::NewGuid();
		}
	}
}

void UFVInventoryComponent::HandleStackReceived(const FFVItemStack& Stack, AActor* Source)
{
	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->ReportTaken(AddStackInParts(Stack));
	}
}

int32 UFVInventoryComponent::AddStackInParts(FFVItemStack Stack)
{
	if (!Stack.Item || Stack.Quantity <= 0)
	{
		return 0;
	}

	int32 Taken = 0;

	// A partly used unit goes in alone and first; the units after it are full.
	if (Stack.Quantity > 1 && Stack.FindData<FFVItemInstance_Charges>())
	{
		FFVItemStack Used = Stack;
		Used.Quantity = 1;
		if (!AddStack(Used).IsValid())
		{
			return 0;
		}

		Taken = 1;
		--Stack.Quantity;
		Stack.RemoveData<FFVItemInstance_Charges>();
	}

	if (Stack.InstanceData.IsEmpty())
	{
		return Taken + AddItem(Stack.Item, Stack.Quantity);
	}

	// Stacks with instance data never merge, so they go in as whole stacks of at most the item's stack size.
	const int32 MaxPerStack = Stack.Item->MaxQuantity;
	while (Stack.Quantity > 0)
	{
		FFVItemStack Part = Stack;
		Part.Quantity = MaxPerStack <= 0 ? Stack.Quantity : FMath::Min(Stack.Quantity, MaxPerStack);
		if (!AddStack(Part).IsValid())
		{
			break;
		}

		Taken += Part.Quantity;
		Stack.Quantity -= Part.Quantity;
	}
	return Taken;
}

void UFVInventoryComponent::HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem)
{
	RefreshContainers();
}

void UFVInventoryComponent::HandleEquipmentReset()
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
		if (Stack.Item != nullptr && !IsKeyStack(Stack))
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
		if (Stack.Item != nullptr && !IsKeyStack(Stack))
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

bool UFVInventoryComponent::FitsAfterChange(const float WeightLimitChange, const int32 CellCapacityChange, const float WeightChange, const int32 UsedCellChange) const
{
	if (!bApplyCarryLimits)
	{
		return true;
	}

	const UFVInventorySettings& Settings = UFVInventorySettings::Get();

	if (WeightLimitChange < 0.f && IsEnforced(CVarEnforceWeight, Settings.bEnforceWeight)
		&& GetTotalWeight() + WeightChange > GetWeightLimit() + WeightLimitChange + KINDA_SMALL_NUMBER)
	{
		return false;
	}

	if (CellCapacityChange < 0 && IsEnforced(CVarEnforceSpace, Settings.bEnforceSpace)
		&& GetUsedCells() + UsedCellChange > GetCellCapacity() + CellCapacityChange)
	{
		return false;
	}

	return true;
}

int32 UFVInventoryComponent::GetAcceptedQuantity(const UFVItemDefinition* Item, const int32 Quantity) const
{
	if (Item == nullptr || Quantity <= 0)
	{
		return 0;
	}

	if (!bApplyCarryLimits || Item->IsKeyItem())
	{
		return Quantity;
	}

	const UFVInventorySettings& Settings = UFVInventorySettings::Get();
	int32 Accepted = Quantity;

	if (IsEnforced(CVarEnforceWeight, Settings.bEnforceWeight) && Item->Weight > 0.f)
	{
		const float Room = GetWeightLimit() - GetTotalWeight();
		Accepted = FMath::Clamp(FMath::FloorToInt32(Room / Item->Weight), 0, Accepted);
	}

	if (Accepted <= 0 || !IsEnforced(CVarEnforceSpace, Settings.bEnforceSpace))
	{
		return Accepted;
	}

	// Units first fill existing stacks for free; every new stack claims the footprint.
	const int32 MaxPerStack = Item->MaxQuantity;
	int64 Room = 0;
	for (const FFVItemStack& Stack : Items)
	{
		if (CanMerge(Stack, Item))
		{
			if (MaxPerStack <= 0)
			{
				return Accepted;
			}
			Room += FMath::Max(0, MaxPerStack - Stack.Quantity);
		}
	}

	if (Accepted <= Room)
	{
		return Accepted;
	}

	const int32 NewStacks = FMath::Max(0, GetCellCapacity() - GetUsedCells()) / Item->GetCellCount();
	const int64 NewRoom = MaxPerStack <= 0 ? (NewStacks > 0 ? Accepted : 0) : int64(NewStacks) * MaxPerStack;
	return static_cast<int32>(FMath::Min<int64>(Accepted, Room + NewRoom));
}

int32 UFVInventoryComponent::AddItem(UFVItemDefinition* Item, const int32 Quantity)
{
	return AddUnits(Item, GetAcceptedQuantity(Item, Quantity));
}

int32 UFVInventoryComponent::AddStoryItem(UFVItemDefinition* Item, const int32 Quantity)
{
	return Item ? AddUnits(Item, Quantity) : 0;
}

int32 UFVInventoryComponent::AddUnits(UFVItemDefinition* Item, const int32 Added)
{
	if (Added <= 0)
	{
		return 0;
	}

	const int32 OldTotal = GetQuantity(Item);
	const int32 MaxPerStack = Item->MaxQuantity;
	int32 Left = Added;
	TArray<FGuid> Changed;

	for (FFVItemStack& Stack : Items)
	{
		if (Left > 0 && CanMerge(Stack, Item))
		{
			const int32 Taken = MaxPerStack <= 0 ? Left : FMath::Min(Left, MaxPerStack - Stack.Quantity);
			if (Taken > 0)
			{
				Stack.Quantity += Taken;
				Left -= Taken;
				Changed.Add(Stack.InstanceId);
			}
		}
	}

	while (Left > 0)
	{
		FFVItemStack& NewStack = Items.AddDefaulted_GetRef();
		NewStack.InstanceId = FGuid::NewGuid();
		NewStack.Item = Item;
		NewStack.Quantity = MaxPerStack <= 0 ? Left : FMath::Min(Left, MaxPerStack);
		Left -= NewStack.Quantity;
		Changed.Add(NewStack.InstanceId);
	}

	for (const FGuid& Id : Changed)
	{
		OnStackChanged.Broadcast(Id);
	}
	BroadcastItemChanged(Item, OldTotal);
	return Added;
}

bool UFVInventoryComponent::CanAcceptStack(const FFVItemStack& Stack) const
{
	const UFVItemDefinition* Item = Stack.Item;
	if (Item == nullptr || Stack.Quantity <= 0 || (Item->MaxQuantity > 0 && Stack.Quantity > Item->MaxQuantity))
	{
		return false;
	}

	if (!bApplyCarryLimits || Item->IsKeyItem())
	{
		return true;
	}

	const UFVInventorySettings& Settings = UFVInventorySettings::Get();
	if (IsEnforced(CVarEnforceWeight, Settings.bEnforceWeight)
		&& GetTotalWeight() + Item->Weight * Stack.Quantity > GetWeightLimit() + KINDA_SMALL_NUMBER)
	{
		return false;
	}

	return !IsEnforced(CVarEnforceSpace, Settings.bEnforceSpace) || GetUsedCells() + Item->GetCellCount() <= GetCellCapacity();
}

FGuid UFVInventoryComponent::AddStack(FFVItemStack Stack)
{
	if (!CanAcceptStack(Stack))
	{
		return FGuid();
	}

	UFVItemDefinition* Item = Stack.Item;
	const int32 OldTotal = GetQuantity(Item);
	Stack.InstanceId = FGuid::NewGuid();
	Items.Add(Stack);

	OnStackChanged.Broadcast(Stack.InstanceId);
	BroadcastItemChanged(Item, OldTotal);
	return Stack.InstanceId;
}

bool UFVInventoryComponent::RemoveItem(UFVItemDefinition* Item, const int32 Quantity)
{
	if (!Item || Quantity <= 0 || GetQuantity(Item) < Quantity)
	{
		return false;
	}

	// Plain stacks before customized ones, and worn ones last.
	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner());
	auto Priority = [Equipment](const FFVItemStack& Stack)
	{
		const bool bWorn = Equipment && Equipment->IsInstanceEquipped(Stack.InstanceId);
		return (bWorn ? 2 : 0) + (Stack.InstanceData.IsEmpty() ? 0 : 1);
	};

	TArray<const FFVItemStack*> Candidates;
	for (const FFVItemStack& Stack : Items)
	{
		if (Stack.Item == Item)
		{
			Candidates.Add(&Stack);
		}
	}
	Algo::StableSortBy(Candidates, [&Priority](const FFVItemStack* Stack) { return Priority(*Stack); });

	TArray<TPair<FGuid, int32>> Plan;
	int32 Left = Quantity;
	for (const FFVItemStack* Stack : Candidates)
	{
		if (Left <= 0)
		{
			break;
		}

		const int32 Taken = FMath::Min(Left, Stack->Quantity);
		if (!CanRemoveFromStack(*Stack, Taken))
		{
			return false;
		}
		Plan.Emplace(Stack->InstanceId, Stack->Quantity - Taken);
		Left -= Taken;
	}

	const int32 OldTotal = GetQuantity(Item);
	for (const TPair<FGuid, int32>& Step : Plan)
	{
		SetStackQuantity(Step.Key, Step.Value);
	}
	BroadcastItemChanged(Item, OldTotal);
	return true;
}

bool UFVInventoryComponent::RemoveStack(const FGuid InstanceId, const int32 Quantity)
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	if (!Stack || Quantity <= 0 || Stack->Quantity < Quantity || !CanRemoveFromStack(*Stack, Quantity))
	{
		return false;
	}

	UFVItemDefinition* Item = Stack->Item;
	const int32 OldTotal = GetQuantity(Item);
	SetStackQuantity(InstanceId, Stack->Quantity - Quantity);
	BroadcastItemChanged(Item, OldTotal);
	return true;
}

bool UFVInventoryComponent::CanRemoveFromStack(const FFVItemStack& Stack, const int32 Quantity) const
{
	if (Quantity < Stack.Quantity || Stack.Item == nullptr)
	{
		return true;
	}

	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner());
	const FFVItemFragment_Container* Container = Stack.Item->FindFragment<FFVItemFragment_Container>();
	if (!Container || !Equipment || !Equipment->IsInstanceEquipped(Stack.InstanceId))
	{
		return true;
	}

	// Dropping a worn bag loses its allowance but also its own weight and footprint.
	return FitsAfterChange(-Container->WeightBonus, -Container->Cells, -Stack.Item->Weight * Stack.Quantity, -Stack.Item->GetCellCount());
}

bool UFVInventoryComponent::SetInstanceData(const FFVItemStack& Stack)
{
	FFVItemStack* Existing = FindMutableStack(Stack.InstanceId);
	if (!Existing)
	{
		return false;
	}

	Existing->InstanceData = Stack.InstanceData;
	OnStackChanged.Broadcast(Stack.InstanceId);
	return true;
}

int32 UFVInventoryComponent::GetQuantity(const UFVItemDefinition* Item) const
{
	int32 Total = 0;
	for (const FFVItemStack& Stack : Items)
	{
		if (Item && Stack.Item == Item)
		{
			Total += Stack.Quantity;
		}
	}
	return Total;
}

bool UFVInventoryComponent::GetStack(const FGuid InstanceId, FFVItemStack& OutStack) const
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	if (Stack)
	{
		OutStack = *Stack;
	}
	return Stack != nullptr;
}

const FFVItemStack* UFVInventoryComponent::FindStack(const FGuid& InstanceId) const
{
	return InstanceId.IsValid() ? Items.FindByPredicate([&InstanceId](const FFVItemStack& Entry) { return Entry.InstanceId == InstanceId; }) : nullptr;
}

FFVItemStack* UFVInventoryComponent::FindMutableStack(const FGuid& InstanceId)
{
	return const_cast<FFVItemStack*>(FindStack(InstanceId));
}

TArray<FFVItemStack> UFVInventoryComponent::GetItemsInCategory(const FGameplayTag Category) const
{
	return Items.FilterByPredicate([&Category](const FFVItemStack& Entry)
	{
		return Entry.Item && Entry.Item->Category.MatchesTag(Category);
	});
}

TArray<FFVItemStack> UFVInventoryComponent::GetStorageItems() const
{
	return Items.FilterByPredicate([](const FFVItemStack& Entry) { return Entry.Item && !Entry.Item->IsKeyItem(); });
}

TArray<FFVItemStack> UFVInventoryComponent::GetKeyItems() const
{
	return Items.FilterByPredicate([](const FFVItemStack& Entry) { return IsKeyStack(Entry); });
}

bool UFVInventoryComponent::FindItemWithTag(const FGameplayTag Tag, FFVItemStack& OutStack) const
{
	const FFVItemStack* Found = Tag.IsValid()
		? Items.FindByPredicate([&Tag](const FFVItemStack& Entry) { return Entry.Item && Entry.Item->Tags.HasTag(Tag); })
		: nullptr;

	if (Found)
	{
		OutStack = *Found;
	}
	return Found != nullptr;
}

int32 UFVInventoryComponent::GetQuantityWithTag(const FGameplayTag Tag) const
{
	int32 Total = 0;
	for (const FFVItemStack& Stack : Items)
	{
		if (Tag.IsValid() && Stack.Item && Stack.Item->Tags.HasTag(Tag))
		{
			Total += Stack.Quantity;
		}
	}
	return Total;
}

FGuid UFVInventoryComponent::FindStackToUse(const UFVItemDefinition* Item) const
{
	// A partly used unit first, then plain stacks before customized ones, and worn ones last.
	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner());
	const FFVItemStack* Best = nullptr;
	int32 BestPriority = MAX_int32;

	for (const FFVItemStack& Stack : Items)
	{
		if (Item == nullptr || Stack.Item != Item)
		{
			continue;
		}

		const bool bWorn = Equipment && Equipment->IsInstanceEquipped(Stack.InstanceId);
		const int32 Kind = Stack.FindData<FFVItemInstance_Charges>() ? 0 : (Stack.InstanceData.IsEmpty() ? 1 : 2);
		const int32 Priority = (bWorn ? 3 : 0) + Kind;
		if (Priority < BestPriority)
		{
			Best = &Stack;
			BestPriority = Priority;
		}
	}

	return Best ? Best->InstanceId : FGuid();
}

bool UFVInventoryComponent::CanUseItem(const UFVItemDefinition* Item) const
{
	return CanUseStack(FindStackToUse(Item));
}

bool UFVInventoryComponent::UseItem(UFVItemDefinition* Item)
{
	return UseStack(FindStackToUse(Item));
}

bool UFVInventoryComponent::CanUseStack(const FGuid InstanceId) const
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	const FFVItemFragment_Usable* Usable = Stack && Stack->Item ? Stack->Item->FindFragment<FFVItemFragment_Usable>() : nullptr;
	if (!Usable)
	{
		return false;
	}

	// A unit this use would use up must be able to leave the inventory.
	FFVItemStack Spent = *Stack;
	if (UFVItemStackLibrary::SpendUse(Spent) && !CanRemoveFromStack(*Stack, 1))
	{
		return false;
	}

	return Usable->UseConditions.Evaluate(UFVConditionLibrary::MakeContext(GetOwner(), GetOwner()));
}

bool UFVInventoryComponent::UseStack(const FGuid InstanceId)
{
	if (!CanUseStack(InstanceId))
	{
		return false;
	}

	UFVItemDefinition* Item = FindStack(InstanceId)->Item;
	const FFVItemFragment_Usable* Usable = Item->FindFragment<FFVItemFragment_Usable>();
	Usable->Effects.Apply(UFVConditionLibrary::MakeContext(GetOwner(), GetOwner()));

	// Effects may have changed the inventory, so the stack is looked up again.
	SpendUse(InstanceId);

	OnItemUsed.Broadcast(Item);
	return true;
}

void UFVInventoryComponent::SpendUse(const FGuid& InstanceId)
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	if (!Stack || !Stack->Item)
	{
		return;
	}

	const bool bCharges = Stack->Item->FindFragment<FFVItemFragment_Charges>() != nullptr;
	FFVItemStack Spent = *Stack;

	if (UFVItemStackLibrary::SpendUse(Spent))
	{
		RemoveStack(InstanceId, 1);
	}
	else if (bCharges && Spent.Quantity > 1)
	{
		// The partly used unit keeps the stack's id, so equipment and callers holding the id go on draining it;
		// the full units left become a stack of their own.
		FFVItemStack Full = Spent;
		Full.RemoveData<FFVItemInstance_Charges>();
		Full.InstanceId = FGuid::NewGuid();
		Full.Quantity = Spent.Quantity - 1;

		FFVItemStack* Used = FindMutableStack(InstanceId);
		Used->InstanceData = Spent.InstanceData;
		Used->Quantity = 1;

		Items.Add(Full);
		OnStackChanged.Broadcast(InstanceId);
		OnStackChanged.Broadcast(Full.InstanceId);
		OnCapacityChanged.Broadcast();
		return;
	}

	// The unit's Charges Left changed, or was cleared from the full units left.
	FFVItemStack* Rest = bCharges ? FindMutableStack(InstanceId) : nullptr;
	if (Rest)
	{
		Rest->InstanceData = Spent.InstanceData;
		OnStackChanged.Broadcast(InstanceId);
	}
}

int32 UFVInventoryComponent::GetCharges(const FGuid InstanceId) const
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	return Stack ? UFVItemStackLibrary::GetChargesLeft(*Stack) : 0;
}

int32 UFVInventoryComponent::TransferStack(UFVInventoryComponent* To, const FGuid InstanceId, const int32 Quantity)
{
	return MoveStack(To, InstanceId, Quantity, false, FGameplayTag());
}

int32 UFVInventoryComponent::TransferStackAsStolen(UFVInventoryComponent* To, const FGuid InstanceId, const int32 Quantity, const FGameplayTag StolenFrom)
{
	return MoveStack(To, InstanceId, Quantity, true, StolenFrom);
}

int32 UFVInventoryComponent::MoveStack(UFVInventoryComponent* To, const FGuid& InstanceId, const int32 Quantity, const bool bMarkStolen, const FGameplayTag& StolenFrom)
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner());
	if (!To || To == this || !Stack || !Stack->Item || Quantity <= 0 || (Equipment && Equipment->IsInstanceEquipped(InstanceId)))
	{
		return 0;
	}

	FFVItemStack Moved = *Stack;
	Moved.Quantity = FMath::Min(Quantity, Stack->Quantity);
	if (bMarkStolen)
	{
		Moved.FindOrAddData<FFVItemInstance_Stolen>().Owner = StolenFrom;
	}

	// Plain units merge and may move in part; a stack with instance data moves whole, up to what one stack holds.
	const bool bPlain = Moved.InstanceData.IsEmpty();
	if (bPlain)
	{
		Moved.Quantity = To->GetAcceptedQuantity(Moved.Item, Moved.Quantity);
	}
	else if (Moved.Item->MaxQuantity > 0)
	{
		Moved.Quantity = FMath::Min(Moved.Quantity, Moved.Item->MaxQuantity);
	}

	if (Moved.Quantity <= 0 || (!bPlain && !To->CanAcceptStack(Moved)) || !RemoveStack(InstanceId, Moved.Quantity))
	{
		return 0;
	}

	if (bPlain)
	{
		To->AddItem(Moved.Item, Moved.Quantity);
	}
	else
	{
		To->AddStack(Moved);
	}
	return Moved.Quantity;
}

int32 UFVInventoryComponent::TransferAll(UFVInventoryComponent* To, const bool bIncludeMoney, const FGameplayTag StolenFrom)
{
	if (!To || To == this)
	{
		return 0;
	}

	const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(GetOwner());
	TArray<TPair<FGuid, int32>> Unworn;
	for (const FFVItemStack& Stack : Items)
	{
		if (!Equipment || !Equipment->IsInstanceEquipped(Stack.InstanceId))
		{
			Unworn.Emplace(Stack.InstanceId, Stack.Quantity);
		}
	}

	int32 MovedStacks = 0;
	for (const TPair<FGuid, int32>& Entry : Unworn)
	{
		if (MoveStack(To, Entry.Key, Entry.Value, StolenFrom.IsValid(), StolenFrom) >= Entry.Value)
		{
			++MovedStacks;
		}
	}

	if (bIncludeMoney)
	{
		TransferMoney(To, Money);
	}
	return MovedStacks;
}

void UFVInventoryComponent::AddMoney(const int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	const int32 OldMoney = Money;
	Money = static_cast<int32>(FMath::Min<int64>(int64(Money) + Amount, MAX_int32));
	OnMoneyChanged.Broadcast(OldMoney, Money);
}

bool UFVInventoryComponent::RemoveMoney(const int32 Amount)
{
	if (Amount < 0 || Money < Amount)
	{
		return false;
	}

	if (Amount > 0)
	{
		const int32 OldMoney = Money;
		Money -= Amount;
		OnMoneyChanged.Broadcast(OldMoney, Money);
	}
	return true;
}

int32 UFVInventoryComponent::TransferMoney(UFVInventoryComponent* To, const int32 Amount)
{
	const int32 Moved = FMath::Min(Amount, Money);
	if (!To || To == this || Moved <= 0)
	{
		return 0;
	}

	RemoveMoney(Moved);
	To->AddMoney(Moved);
	return Moved;
}

void UFVInventoryComponent::SetStackQuantity(const FGuid& InstanceId, const int32 NewQuantity)
{
	FFVItemStack* Stack = FindMutableStack(InstanceId);
	if (!Stack || Stack->Quantity == NewQuantity)
	{
		return;
	}

	if (NewQuantity <= 0)
	{
		Items.RemoveAll([&InstanceId](const FFVItemStack& Entry) { return Entry.InstanceId == InstanceId; });
	}
	else
	{
		Stack->Quantity = NewQuantity;
	}

	OnStackChanged.Broadcast(InstanceId);
}

void UFVInventoryComponent::BroadcastItemChanged(UFVItemDefinition* Item, const int32 OldTotal)
{
	OnItemChanged.Broadcast(Item, OldTotal, GetQuantity(Item));
	OnCapacityChanged.Broadcast();
}
