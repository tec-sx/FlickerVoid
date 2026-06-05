#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FVWorldObjectBase.generated.h"

UCLASS()
class FLICKERVOIDWORLD_API AFVWorldObjectBase : public AActor
{
	GENERATED_BODY()

public:
	AFVWorldObjectBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification")
	FName ObjectID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification")
	FText Description;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "WorldObject")
	UStaticMeshComponent* GetMeshComponent() const;
};
