#include "Inventory/FVInventoryComponent.h"

#include "Algo/StableSort.h"
#include "Conditions/FVConditionLibrary.h"
#include "Engine/World.h"
#include "Equipment/FVEquipmentComponent.h"
#include "FVInventorySettings.h"
#include "FVInventorySystem.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/Actor.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"
#include "Pickup/FVItemReceiverComponent.h"
#include "Pickup/FVPickupComponent.h"
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
		Receiver->OnItemReceived.AddUniqueDynamic(this, &UFVInventoryComponent::HandleItemReceived);
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

bool UFVInventoryComponent::FitsAfterChange(const float WeightLimitChange, const int32 CellCapacityChange, const float WeightChange, const int32 UsedCellChange) const
{
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
	const int32 Added = GetAcceptedQuantity(Item, Quantity);
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

FGuid UFVInventoryComponent::AddStack(FFVItemStack Stack)
{
	UFVItemDefinition* Item = Stack.Item;
	if (Item == nullptr || Stack.Quantity <= 0 || (Item->MaxQuantity > 0 && Stack.Quantity > Item->MaxQuantity))
	{
		return FGuid();
	}

	const UFVInventorySettings& Settings = UFVInventorySettings::Get();
	if (IsEnforced(CVarEnforceWeight, Settings.bEnforceWeight)
		&& GetTotalWeight() + Item->Weight * Stack.Quantity > GetWeightLimit() + KINDA_SMALL_NUMBER)
	{
		return FGuid();
	}

	if (IsEnforced(CVarEnforceSpace, Settings.bEnforceSpace) && GetUsedCells() + Item->GetCellCount() > GetCellCapacity())
	{
		return FGuid();
	}

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

bool UFVInventoryComponent::CanDropStack(const FGuid InstanceId, const int32 Quantity) const
{
	const FFVItemStack* Stack = FindStack(InstanceId);
	return Stack && Stack->Item && !Stack->Item->bQuestItem && Quantity > 0 && Stack->Quantity >= Quantity
		&& CanRemoveFromStack(*Stack, Quantity);
}

AActor* UFVInventoryComponent::DropStack(const FGuid InstanceId, const int32 Quantity)
{
	if (!CanDropStack(InstanceId, Quantity))
	{
		return nullptr;
	}

	UFVItemDefinition* Item = FindStack(InstanceId)->Item;
	const FFVItemFragment_Pickup* PickupData = Item->FindFragment<FFVItemFragment_Pickup>();
	const TSoftClassPtr<AActor> SoftClass = PickupData && !PickupData->PickupClass.IsNull() ? PickupData->PickupClass : UFVInventorySettings::Get().DefaultPickupClass;
	UClass* PickupClass = SoftClass.LoadSynchronous();
	UWorld* World = GetWorld();
	if (!PickupClass || !World)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: no pickup class to drop %s; set Default Pickup Class in the inventory settings."), *GetNameSafe(GetOwner()), *GetNameSafe(Item));
		return nullptr;
	}

	// Spawned fully (not deferred) so components added in Blueprint exist before we look for the pickup component.
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AActor* Pickup = World->SpawnActor<AActor>(PickupClass, GetDropTransform(), SpawnParams);
	UFVPickupComponent* PickupComponent = Pickup ? Pickup->FindComponentByClass<UFVPickupComponent>() : nullptr;
	if (!PickupComponent)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: could not drop %s; %s has no FV Pickup Component."), *GetNameSafe(GetOwner()), *GetNameSafe(Item), *GetNameSafe(PickupClass));
		if (Pickup)
		{
			Pickup->Destroy();
		}
		return nullptr;
	}

	PickupComponent->SetItem(Item, Quantity);
	RemoveStack(InstanceId, Quantity);
	OnItemDropped.Broadcast(Item, Quantity, Pickup);
	return Pickup;
}

FTransform UFVInventoryComponent::GetDropTransform() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FTransform::Identity;
	}

	const FVector Forward = Owner->GetActorForwardVector().GetSafeNormal2D();
	FVector Location = Owner->GetActorLocation() + Forward * UFVInventorySettings::Get().DropDistance;

	// Land on whatever is below the drop point rather than in the air at the owner's centre.
	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(FVInventoryDrop), false, Owner);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Location + FVector(0.f, 0.f, 50.f), Location - FVector(0.f, 0.f, 500.f), ECC_Visibility, Params))
	{
		Location = Hit.ImpactPoint;
	}

	return FTransform(Forward.Rotation(), Location);
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
