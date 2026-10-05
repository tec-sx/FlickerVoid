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

	UFUNCTION(BlueprintCallable, Category = "Item")
	UE_API bool ReceiveItem(UFVItemDefinition* Item, int32 Quantity, AActor* Source);

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
