#pragma once

#include "Components/ActorComponent.h"
#include "FVPickupComponent.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UFVItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnPickedUp, AActor*, Picker);

/** Makes the owning actor collectable into the picker's UFVItemReceiverComponent. */
UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVPickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVPickupComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	UE_API bool TryPickup(AActor* Picker);

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UE_API bool CanBePickedUpBy(AActor* Picker) const;

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UFVItemDefinition* GetItem() const { return Item; }

	UFUNCTION(BlueprintPure, Category = "Pickup")
	int32 GetQuantity() const { return Quantity; }

	UPROPERTY(BlueprintAssignable, Category = "Pickup")
	FFVOnPickedUp OnPickedUp;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOwnerOnPickup = true;
};

#undef UE_API
