#pragma once

#include "Components/ActorComponent.h"
#include "FVPickupComponent.generated.h"

#define UE_API FVINVENTORYSYSTEM_API

class UFVItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnPickedUp, AActor*, Picker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnPickupItemChanged);

/** Makes the owning actor collectable into the picker's UFVItemReceiverComponent. */
UCLASS(MinimalAPI, ClassGroup=(FV), meta=(BlueprintSpawnableComponent))
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

	UFUNCTION(BlueprintCallable, Category = "Pickup", meta = (ToolTip = "Changes what the pickup holds, e.g. on an actor spawned for a dropped item."))
	UE_API void SetItem(UFVItemDefinition* InItem, int32 InQuantity = 1);

	UPROPERTY(BlueprintAssignable, Category = "Pickup")
	FFVOnPickedUp OnPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "Pickup")
	FFVOnPickupItemChanged OnItemChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOwnerOnPickup = true;
};

#undef UE_API
