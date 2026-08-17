#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Interactions/FVInteractionConfig.h"
#include "FVInteractionSet.generated.h"

UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (TitleProperty = AbilityTag))
	TArray<FFVInteractionConfig> Interactions;
};
