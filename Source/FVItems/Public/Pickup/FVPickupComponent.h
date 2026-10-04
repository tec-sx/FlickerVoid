#pragma once

#include "Components/ActorComponent.h"
#include "FVPickupComponent.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UFVInventoryItemTemplate;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnPickedUp, AActor*, Picker);

/**
 * Makes the owning actor collectable into an FVInventoryEquipmentSystem inventory.
 * The pickup ability calls TryPickup on the interactable's owner when its montage finishes.
 */
UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVPickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVPickupComponent() { PrimaryComponentTick.bCanEverTick = false; }

	/** Adds the item to the picker's inventory and, on success, removes the owner from the world. */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	UE_API bool TryPickup(AActor* Picker);

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UE_API bool CanBePickedUpBy(AActor* Picker) const;

	/** Finds an inventory on the actor, or on its controller or player state when the actor is a pawn. */
	UFUNCTION(BlueprintPure, Category = "Pickup")
	static UE_API UObject* FindInventory(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UFVInventoryItemTemplate* GetItemTemplate() const { return ItemTemplate; }

	UPROPERTY(BlueprintAssignable, Category = "Pickup")
	FFVOnPickedUp OnPickedUp;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UFVInventoryItemTemplate> ItemTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Durability = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOwnerOnPickup = true;
};

#undef UE_API
