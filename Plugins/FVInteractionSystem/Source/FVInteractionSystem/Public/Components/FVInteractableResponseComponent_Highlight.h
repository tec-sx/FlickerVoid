#pragma once

#include "Core/FVInteractionTypes.h"
#include "CoreMinimal.h"
#include "FVInteractableResponseComponent.h"
#include "FVInteractionSystemSettings.h"

#include "FVInteractableResponseComponent_Highlight.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UFVInteractableResponseComponent_Highlight final : public UFVInteractableResponseComponent
{
	GENERATED_BODY()

protected:
	virtual void BindEvents_Implementation(UFVInteractableComponent* Ineractable) override;
	virtual void UnbindEvents_Implementation(UFVInteractableComponent* Interactable) override;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight")
	EFVHighlightSetupType SetupType = EFVHighlightSetupType::Default;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight", meta = (ShowOnlyInnerProperties))
	FFVInteractionHighlightSetup HighlightSetup;

private:
	UFUNCTION()
	void OnFocusStateChanged(bool bIsInFocus, UFVInteractorComponent* Interactor);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> HighlightTargets;

	TArray<bool> CachedRenderCustomDepth;
	TArray<int32> CachedStencilValues;
};
