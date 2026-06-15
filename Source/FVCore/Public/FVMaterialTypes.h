#pragma once

#include "CoreMinimal.h"
#include "FVMaterialTypes.generated.h"

USTRUCT(BlueprintType)
struct FLICKERVOIDCORE_API FFVMaterialOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	UMaterialInstance* OverrideMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	int32 MaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	float TintIntensity = 1.0f;
};