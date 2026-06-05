#pragma once

#include "CoreMinimal.h"
#include "FVLegalStructures.generated.h"

USTRUCT(BlueprintType)
struct FVCredits
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legal")
	FString AssetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legal")
	FString CreatorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legal")
	TArray<FString> Clauses;
};