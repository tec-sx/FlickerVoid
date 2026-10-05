#include "FVInventoryDebugSubsystem.h"

#include "Equipment/FVEquipmentComponent.h"
#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryDebugSubsystem)

static TAutoConsoleVariable<bool> CVarInventoryDebugHUD(
	TEXT("FVCvar.Inventory.Debug.HUD"),
	false,
	TEXT("Show the player's inventory, weight and grid space on screen."));

bool UFVInventoryDebugSubsystem::IsEnabled() const
{
	return CVarInventoryDebugHUD.GetValueOnGameThread();
}

void UFVInventoryDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	APawn* Pawn = FVDebug::GetPlayerPawn(GetWorld());
	const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Pawn);
	if (Inventory == nullptr)
	{
		return;
	}

	OutLines.Add(FString::Printf(TEXT("%.1f / %.1f kg, %d / %d cells"),
		Inventory->GetTotalWeight(), Inventory->GetWeightLimit(), Inventory->GetUsedCells(), Inventory->GetCellCapacity()));

	for (const FFVItemStack& Stack : Inventory->GetItems())
	{
		if (Stack.Item != nullptr)
		{
			OutLines.Add(FString::Printf(TEXT("  %s x%d"), *Stack.Item->GetName(), Stack.Quantity));
		}
	}

	if (const UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(Pawn))
	{
		for (const FFVEquippedItem& Entry : Equipment->GetAllEquipped())
		{
			OutLines.Add(FString::Printf(TEXT("  [%s] %s"), *Entry.Slot.ToString(), *GetNameSafe(Entry.Item)));
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdGiveItem(
	TEXT("FV.Inventory.Give"),
	TEXT("FV.Inventory.Give <ItemIdOrAsset> [Quantity] - give the player an item."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(FVDebug::GetPlayerPawn(World));
		UFVItemDefinition* Item = Args.Num() > 0 ? FVDebug::FindDefinition<UFVItemDefinition>(Args[0]) : nullptr;

		if (Inventory == nullptr || Item == nullptr)
		{
			return;
		}

		const int32 Requested = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 1;
		const int32 Added = Inventory->AddItem(Item, Requested);
		UE_LOG(LogFVDebug, Display, TEXT("Added %d of %d %s."), Added, Requested, *Item->GetName());
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdEquipItem(
	TEXT("FV.Inventory.Equip"),
	TEXT("FV.Inventory.Equip <ItemIdOrAsset> - equip an item the player carries."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVEquipmentComponent* Equipment = UFVEquipmentComponent::Find(FVDebug::GetPlayerPawn(World));
		UFVItemDefinition* Item = Args.Num() > 0 ? FVDebug::FindDefinition<UFVItemDefinition>(Args[0]) : nullptr;

		if (Equipment != nullptr && Item != nullptr && !Equipment->Equip(Item))
		{
			UE_LOG(LogFVDebug, Warning, TEXT("Could not equip %s."), *Item->GetName());
		}
	}));
