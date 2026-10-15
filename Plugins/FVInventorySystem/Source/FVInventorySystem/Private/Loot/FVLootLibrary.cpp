#include "Loot/FVLootLibrary.h"

#include "Conditions/FVConditionLibrary.h"
#include "FVInventorySystem.h"
#include "GameFramework/Actor.h"
#include "Items/FVItemDefinition.h"
#include "Loot/FVLootTableDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLootLibrary)

namespace FVLootRoll
{
	constexpr int32 MaxDepth = 8;

	struct FRoll
	{
		FFVConditionContext Context;
		FFVLootResult Result;
		bool bTooDeep = false;
	};

	void AddUnits(FFVLootResult& Result, UFVItemDefinition* Item, const int32 Quantity)
	{
		if (FFVItemStack* Existing = Result.Stacks.FindByPredicate([Item](const FFVItemStack& Stack) { return Stack.Item == Item; }))
		{
			Existing->Quantity += Quantity;
			return;
		}

		FFVItemStack& Stack = Result.Stacks.AddDefaulted_GetRef();
		Stack.Item = Item;
		Stack.Quantity = Quantity;
	}

	void RollTable(const UFVLootTableDefinition& Table, const int32 Depth, FRoll& Roll)
	{
		if (Depth >= MaxDepth)
		{
			Roll.bTooDeep = true;
			return;
		}

		const int32 MinMoney = FMath::Max(0, Table.MinMoney);
		Roll.Result.Money += FMath::RandRange(MinMoney, FMath::Max(MinMoney, Table.MaxMoney));

		for (const FFVLootEntry& Entry : Table.Entries)
		{
			// FRand can return 1, so a sure entry skips the roll.
			if ((!Entry.Item && !Entry.Table) || (Entry.Chance < 1.f && FMath::FRand() >= Entry.Chance) || !Entry.Conditions.Evaluate(Roll.Context))
			{
				continue;
			}

			const int32 MinQuantity = FMath::Max(1, Entry.MinQuantity);
			const int32 Quantity = FMath::RandRange(MinQuantity, FMath::Max(MinQuantity, Entry.MaxQuantity));

			if (Entry.Table)
			{
				for (int32 Index = 0; Index < Quantity; ++Index)
				{
					RollTable(*Entry.Table, Depth + 1, Roll);
				}
			}
			else
			{
				AddUnits(Roll.Result, Entry.Item, Quantity);
			}
		}
	}
}

FFVLootResult UFVLootLibrary::RollLoot(const UFVLootTableDefinition* Table, AActor* Instigator, AActor* Target)
{
	FVLootRoll::FRoll Roll;
	Roll.Context = UFVConditionLibrary::MakeContext(Instigator, Target);

	if (Table)
	{
		FVLootRoll::RollTable(*Table, 0, Roll);
	}

	if (Roll.bTooDeep)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: loot tables nest more than %d deep; the deeper rolls were skipped."), *GetNameSafe(Table), FVLootRoll::MaxDepth);
	}

	return Roll.Result;
}

int32 UFVLootLibrary::GiveLoot(const UFVLootTableDefinition* Table, UFVInventoryComponent* Inventory, AActor* Instigator)
{
	if (!Table || !Inventory)
	{
		return 0;
	}

	const FFVLootResult Loot = RollLoot(Table, Instigator, Inventory->GetOwner());

	int32 Added = 0;
	for (const FFVItemStack& Stack : Loot.Stacks)
	{
		Added += Inventory->AddItem(Stack.Item, Stack.Quantity);
	}
	Inventory->AddMoney(Loot.Money);
	return Added;
}
