#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/FVInteractionTypes.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"

#include "FVInteractionWidget.generated.h"

class UFVInteractorComponent;

/**
 * Persistent interaction HUD widget. Created once by UFVInteractionUIComponent
 * and fed data through these events. Visibility is left to the implementation.
 */
UCLASS(Abstract, Blueprintable)
class FVINTERACTIONSYSTEM_API UFVInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionInitialized(UFVInteractorComponent* Interactor);

	/** Empty array = nothing to present (no focus, or interactable hides offers). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnOffersChanged(const TArray<FFVInteractionOfferData>& Offers);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnFocusIndicatorChanged(const FSlateBrush& Brush, FGameplayTag InteractableType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnOfferProgress(FGameplayTag ActionTag, float Progress);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnOfferEnded(FGameplayTag ActionTag, bool bSuccess);
};
