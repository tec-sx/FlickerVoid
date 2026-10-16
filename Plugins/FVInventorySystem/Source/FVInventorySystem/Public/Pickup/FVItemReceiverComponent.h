#pragma once

#include "Components/ActorComponent.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVItemReceiverComponent.generated.h"

#define UE_API FVINVENTORYSYSTEM_API

class UFVItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnItemReceived, UFVItemDefinition*, Item, int32, Quantity, AActor*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnStackReceived, const FFVItemStack&, Stack, AActor*, Source);

/** Lets an actor take items from the world; inventories bind to OnStackReceived. */
UCLASS(MinimalAPI, ClassGroup=(FV), meta=(BlueprintSpawnableComponent))
class UFVItemReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVItemReceiverComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintPure, Category = "Item")
	static UE_API UFVItemReceiverComponent* FindReceiver(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (ToolTip = "Offers items to whoever listens and returns how many were taken."))
	UE_API int32 ReceiveItem(UFVItemDefinition* Item, int32 Quantity, AActor* Source);

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (ToolTip = "Offers a stack, instance data included, to whoever listens and returns how many units were taken."))
	UE_API int32 ReceiveStack(const FFVItemStack& Stack, AActor* Source);

	UFUNCTION(BlueprintCallable, Category = "Item", meta = (ToolTip = "Called from an OnStackReceived or OnItemReceived handler to report how much of the offer it took."))
	void ReportTaken(const int32 Quantity)
	{
		TakenQuantity += FMath::Max(Quantity, 0);
		bTakenReported = true;
	}

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsAcceptingItems() const { return bAcceptingItems; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetAcceptingItems(const bool bAccepting) { bAcceptingItems = bAccepting; }

	UPROPERTY(BlueprintAssignable, Category = "Item", meta = (ToolTip = "A stack is offered, instance data included."))
	FFVOnStackReceived OnStackReceived;

	UPROPERTY(BlueprintAssignable, Category = "Item", meta = (ToolTip = "Plain items are offered (stacks without instance data only)."))
	FFVOnItemReceived OnItemReceived;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bAcceptingItems = true;

private:
	int32 TakenQuantity = 0;
	bool bTakenReported = false;
};

#undef UE_API
