#pragma once

#include "CoreMinimal.h"
#include "FVWorldObjectBase.h"
#include "GameplayTagContainer.h"
#include "FVWorldObjectInteractable.generated.h"

UCLASS()
class FLICKERVOIDWORLD_API AFVWorldObjectInteractable : public AFVWorldObjectBase
{
	GENERATED_BODY()

public:
	AFVWorldObjectInteractable();

	// Object Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification", meta = (Categories = "WorldObject"))
	FGameplayTag ObjectTypeTag;
};
