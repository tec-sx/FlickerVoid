#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FVItemPickup.generated.h"

class UFVPickupComponent;
class UStaticMeshComponent;

/**
 * A world item: shows the World Mesh from its item's Pickup fragment and holds an FV Pickup Component. Dropped items
 * spawn as one; make a Blueprint of it to add the interaction components that offer the pickup.
 */
UCLASS(Blueprintable)
class FVINVENTORYSYSTEM_API AFVItemPickup : public AActor
{
	GENERATED_BODY()

public:
	AFVItemPickup();

	UFUNCTION(BlueprintPure, Category = "Pickup")
	UFVPickupComponent* GetPickup() const { return Pickup; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void ApplyItemMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFVPickupComponent> Pickup;
};
