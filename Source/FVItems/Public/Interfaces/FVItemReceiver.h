#pragma once

#include "UObject/Interface.h"
#include "FVItemReceiver.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UFVItemDataAsset;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVItemReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * Anything that can take items from the world: the player's inventory, a container, a quest tracker.
 * Keeps pickups independent of the inventory implementation behind it.
 */
class UE_API IFVItemReceiver
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool CanReceiveItem(const UFVItemDataAsset* Item, int32 Quantity) const;

	/** Returns true when the item was stored. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	bool ReceiveItem(const UFVItemDataAsset* Item, int32 Quantity);

	/** Finds a receiver on the actor itself, its components, or its controller and player state when it is a pawn. */
	static UObject* FindReceiver(AActor* Actor);
};

#undef UE_API
