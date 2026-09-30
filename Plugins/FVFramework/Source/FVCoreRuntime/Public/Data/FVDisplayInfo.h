#pragma once

#include "CoreMinimal.h"
#include "FVDisplayInfo.generated.h"

USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText ShortDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText LongDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FLinearColor Tint = FLinearColor::White;
};
