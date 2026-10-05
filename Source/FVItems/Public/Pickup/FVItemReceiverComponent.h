#pragma once

#include "Components/ActorComponent.h"
#include "FVItemReceiverComponent.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UFVItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnItemReceived, UFVItemDataAsset*, Item, int32, Quantity, AActor*, Source);

/**
 * Marks an actor as able to take items from the world (the player, a container, a quest tracker).
 * Pickups find this component and broadcast OnItemReceived; whatever inventory exists binds to it,
 * so pickups stay independent of the inventory implementation.
 */
UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVItemReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVItemReceiverComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintPure, Category = "Item")
	static UE_API UFVItemReceiverComponent* FindReceiver(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Item")
	UE_API bool ReceiveItem(UFVItemDataAsset* Item, int32 Quantity, AActor* Source);

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsAcceptingItems() const { return bAcceptingItems; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetAcceptingItems(const bool bAccepting) { bAcceptingItems = bAccepting; }

	UPROPERTY(BlueprintAssignable, Category = "Item")
	FFVOnItemReceived OnItemReceived;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bAcceptingItems = true;
};

#undef UE_API
