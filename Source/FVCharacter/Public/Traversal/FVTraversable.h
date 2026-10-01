#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Conditions/FVCondition.h"
#include "Traversal/FVTraversalTypes.h"
#include "FVTraversable.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class UFVTraversableConfigNative;

UCLASS(Blueprintable)
class FLICKERVOIDCHARACTER_API AFVTraversableNative : public AActor
{
GENERATED_BODY()

public:
AFVTraversableNative();

UFUNCTION(BlueprintPure, Category = "Traversal")
FFVLedgeResult GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation) const;

UFUNCTION(BlueprintPure, Category = "Traversal")
bool CanTraverse(AActor* Traverser) const;

protected:
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<UStaticMeshComponent> Mesh;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<USplineComponent> LedgeFront;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<USplineComponent> LedgeLeft;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<USplineComponent> LedgeBack;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<USplineComponent> LedgeRight;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
TObjectPtr<UFVTraversableConfigNative> Config;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
FFVConditionSet Requirements;

private:
USplineComponent* CreateLedge(FName Name);
USplineComponent* FindClosestLedge(const FVector& Location) const;
USplineComponent* GetOpposite(const USplineComponent* Ledge) const;
float GetMinLedgeWidth() const;
float GetLedgeNormalOffset() const;
};