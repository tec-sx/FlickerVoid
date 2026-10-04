#pragma once

#include "Components/ActorComponent.h"
#include "FVPickupComponent.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UFVItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnPickedUp, AActor*, Picker);

/**
 * Makes the owning actor collectable. The item goes to whatever implements IFVItemReceiver
 * on the picker, so the inventory behind it can change without touching pickups.
 * The pickup ability calls TryPickup on the interactable's owner when its montage finishes.
 */
UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVPickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVPickupComponent() { PrimaryComponentTick.bCanEverTick = false; }

	/** Hands the item to the picker's receiver and, on success, removes the owner from the world. */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	UE_API bool TryPickup(AActor* Picker);

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UE_API bool CanBePickedUpBy(AActor* Picker) const;

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UFVItemDataAsset* GetItem() const { return Item; }

	UFUNCTION(BlueprintPure, Category = "Pickup")
	int32 GetQuantity() const { return Quantity; }

	UPROPERTY(BlueprintAssignable, Category = "Pickup")
	FFVOnPickedUp OnPickedUp;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UFVItemDataAsset> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOwnerOnPickup = true;
};

#undef UE_API
