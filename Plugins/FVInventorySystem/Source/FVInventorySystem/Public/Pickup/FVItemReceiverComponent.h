#pragma once

#include "Components/ActorComponent.h"
#include "FVItemReceiverComponent.generated.h"

#define UE_API FVINVENTORYSYSTEM_API

class UFVItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnItemReceived, UFVItemDefinition*, Item, int32, Quantity, AActor*, Source);

/** Lets an actor take items from the world; inventories bind to OnItemReceived. */
UCLASS(MinimalAPI, ClassGroup=(FV), meta=(BlueprintSpawnableComponent))
class UFVItemReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVItemReceiverComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintPure, Category = "Item")
	static UE_API UFVItemReceiverComponent* FindReceiver(AActor* Actor);

	/** Offers items to whoever listens and returns how many were taken. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	UE_API int32 ReceiveItem(UFVItemDefinition* Item, int32 Quantity, AActor* Source);

	/** Called from an OnItemReceived handler to report how much of the offer it took. */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void ReportTaken(const int32 Quantity) { TakenQuantity += FMath::Max(Quantity, 0); }

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsAcceptingItems() const { return bAcceptingItems; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetAcceptingItems(const bool bAccepting) { bAcceptingItems = bAccepting; }

	UPROPERTY(BlueprintAssignable, Category = "Item")
	FFVOnItemReceived OnItemReceived;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bAcceptingItems = true;

private:
	int32 TakenQuantity = 0;
};

#undef UE_API
