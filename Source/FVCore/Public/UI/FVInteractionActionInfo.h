#pragma once
#include "GameplayTagContainer.h"
#include "FVInteractionActionInfo.generated.h"

USTRUCT(BlueprintType)
struct FLICKERVOIDCORE_API FFVInteractionActionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	bool bAvailable = true;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FText UnavailableReason;
};
