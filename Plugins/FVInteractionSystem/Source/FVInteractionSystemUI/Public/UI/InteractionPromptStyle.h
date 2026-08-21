#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "InteractionPromptStyle.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEMUI_API FInteractionPromptStyle
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText RequirementHint;
};


UCLASS(BlueprintType)
class FVINTERACTIONSYSTEMUI_API UInteractionPromptStyleAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction|UI")
	bool FindStyle(FGameplayTag ActionTag, FInteractionPromptStyle& OutStyle) const;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI", meta = (ForceInlineRow, Categories = "Interaction.Action"))
	TMap<FGameplayTag, FInteractionPromptStyle> Styles;
};
