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
	virtual void BindSignals_Implementation(UInteractionSignalComponent* Signal) override;
	virtual void UnbindSignals_Implementation(UInteractionSignalComponent* Signal) override;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight")
	EHighlightSetupType SetupType = EHighlightSetupType::Default;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight", meta = (ShowOnlyInnerProperties))
	FInteractionHighlightSetup HighlightSetup;

private:
	UFUNCTION()
	void OnFocusStateChanged(UInteractableComponent* Interactable);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> HighlightTargets;

	TArray<bool> CachedRenderCustomDepth;
	TArray<int32> CachedStencilValues;
};
