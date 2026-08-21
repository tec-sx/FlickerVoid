#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "InteractionPromptStyle.generated.h"

class UTexture2D;

/** Presentation data for a single interaction action tag. */
USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEMUI_API FInteractionPromptStyle
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	/** Shown instead of DisplayName when the interactor lacks the required ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText RequirementHint;
};

/** Maps interaction action tags to their presentation data. Owned by the UI layer. */
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
