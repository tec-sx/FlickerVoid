#pragma once

#include "CoreMinimal.h"
#include "Interactions/FVInteractionOfferComponent.h"
#include "FVInteractionMessageTypes.generated.h"

/**
 * Domain-side broadcast of the player's current interaction offer.
 * Consumed by the UI router, which translates it into a pure presentation struct.
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionOfferChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<APlayerController> OwningController = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVInteractionOffer Offer;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bHasOffer = false;
};
