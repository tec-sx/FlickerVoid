#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/FVInventoryComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVItemStackLibrary.generated.h"

class UFVItemDefinition;

/** Reads stack instance data and item fragments for Blueprint and script, which can't read instanced structs. Spends uses of a stack for C++. */
UCLASS()
class FVINVENTORYSYSTEM_API UFVItemStackLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "The stack carries the Stolen mark. OutOwner is the faction or character Id it was taken from; empty = unknown owner."))
	static bool IsStolen(const FFVItemStack& Stack, FGameplayTag& OutOwner);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Uses left on the stack's next unit: its Charges Left, else the item's Charges, else 1 for a usable item and 0 otherwise."))
	static int32 GetChargesLeft(const FFVItemStack& Stack);

	/** One use of the stack's next unit, as Use Stack spends it: a charge (kept as Charges Left), or the unit itself after its last charge or with Consume On Use. A used up unit lowers Quantity and returns true. */
	static bool SpendUse(FFVItemStack& Stack);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "Base Price of the item's Value fragment; 0 without one."))
	static int32 GetBasePrice(const UFVItemDefinition* Item);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory", meta = (ToolTip = "The item has a Value fragment marked Sellable."))
	static bool IsSellable(const UFVItemDefinition* Item);
};
