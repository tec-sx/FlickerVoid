#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/FVDisplayInfo.h"
#include "GameplayTagContainer.h"
#include "FVDefinition.generated.h"

UCLASS(Abstract, BlueprintType)
class FVCORERUNTIME_API UFVDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Definition")
	FGameplayTag Id;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Definition")
	FFVDisplayInfo Display;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Definition")
	FGameplayTagContainer Tags;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
