#pragma once

#include "Components/InteractionResponseComponent.h"
#include "Core/InteractionTypes.h"
#include "CoreMinimal.h"

#include "Response_Highlight.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UResponse_Highlight final : public UInteractionResponseComponent
{
	GENERATED_BODY()

protected:
	virtual void BindInteractableResponses_Implementation(UInteractableComponent* Interactable) override;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight")
	EHighlightSetupType SetupType = EHighlightSetupType::Default;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight", meta = (ShowOnlyInnerProperties))
	FInteractionHighlightSetup HighlightSetup;

private:
	UFUNCTION()
	void HandleFocusStateChanged(bool bInFocus, UInteractorComponent* Interactor);

	void CollectTargets();
	void ApplyHighlight(bool bEnabled);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> HighlightTargets;

	TArray<bool> CachedRenderCustomDepth;
	TArray<int32> CachedStencilValues;
};
