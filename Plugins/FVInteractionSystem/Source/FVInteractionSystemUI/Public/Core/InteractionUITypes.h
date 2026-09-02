#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "InteractionUITypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEMUI_API FInteractionSlotStyle : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSlateBrush Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText RequirementHint;
};