#pragma once

#include "Components/ActorComponent.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVPickupComponent.generated.h"

#define UE_API FVINVENTORYSYSTEM_API

class UFVItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnPickedUp, AActor*, Picker);

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

	UFUNCTION(BlueprintPure, Category = "Pickup", meta = (ToolTip = "Item, quantity and instance data as one stack (no instance id)."))
	UE_API FFVItemStack GetStack() const;

	UFUNCTION(BlueprintCallable, Category = "Pickup", meta = (ToolTip = "Sets the item, quantity and instance data, e.g. for a dropped stack."))
	UE_API void SetStack(const FFVItemStack& Stack);

	UFUNCTION(BlueprintPure, Category = "Pickup", meta = (ToolTip = "Everything was taken but the owner was kept (Destroy Owner On Pickup off)."))
	bool IsDepleted() const { return Quantity <= 0; }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetDestroyOwnerOnPickup(const bool bDestroy) { bDestroyOwnerOnPickup = bDestroy; }

	UPROPERTY(BlueprintAssignable, Category = "Pickup", meta = (ToolTip = "Some or all of the stack was taken; Get Quantity is what is left."))
	FFVOnPickedUp OnPickedUp;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Pickup", meta = (ExcludeBaseStruct, ToolTip = "Given to the picked up stack, e.g. a Stolen mark or Charges Left."))
	TArray<TInstancedStruct<FFVItemInstanceFragment>> InstanceData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ToolTip = "Destroy the owner once everything is taken; otherwise the pickup is left depleted."))
	bool bDestroyOwnerOnPickup = true;
};

#undef UE_API
