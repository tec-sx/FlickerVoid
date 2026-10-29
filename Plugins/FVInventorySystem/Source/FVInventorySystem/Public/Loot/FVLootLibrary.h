#pragma once

#include "CoreMinimal.h"
#include "Inventory/FVInventoryComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVLootLibrary.generated.h"

class UFVLootTableDefinition;

/** Items and money rolled from a loot table. */
USTRUCT(BlueprintType)
struct FVINVENTORYSYSTEM_API FFVLootResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Loot", meta = (ToolTip = "One plain stack per item, without instance ids."))
	TArray<FFVItemStack> Stacks;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 Money = 0;
};

/** Rolls loot tables, on their own or into an inventory. */
UCLASS()
class FVINVENTORYSYSTEM_API UFVLootLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Loot", meta = (ToolTip = "Rolls the table and its nested tables. Entry conditions are evaluated with Instigator and Target."))
	static FFVLootResult RollLoot(const UFVLootTableDefinition* Table, AActor* Instigator, AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "FV|Loot", meta = (ToolTip = "Rolls the table with the inventory's owner as Target and adds the items (as far as they fit) and the money. Returns the units added."))
	static int32 GiveLoot(const UFVLootTableDefinition* Table, UFVInventoryComponent* Inventory, AActor* Instigator);
};
