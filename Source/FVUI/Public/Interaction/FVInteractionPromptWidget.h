#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/FVInteractionTypes.h"
#include "FVInteractionPromptWidget.generated.h"

class UFVInteractableComponent;
class UFVInteractionComponent;

//~=============================================================================
// C++ base for the interaction prompt overlay.
//
// Handles all the delegate plumbing (bind/unbind on pawn change) so the
// AngelScript subclass only needs to implement OnFocusChanged().
//
// Usage:
//   1. Subclass in AngelScript, implement OnFocusChanged().
//   2. Assign the subclass to AFVHUD::InteractionPromptWidgetClass.
//   3. The HUD creates it and adds it to the viewport.
//~=============================================================================

UCLASS(Abstract, Blueprintable, BlueprintType)
class FLICKERVOIDUI_API UFVInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Fired whenever focus changes. Interactable is null when the player looks away.
	// Implement this in the AngelScript subclass to update your slots.
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|UI")
	void OnFocusChanged(
		UFVInteractableComponent* FocusedInteractable, 
		const TArray<FFVInteractionActionDisplay>& AvailableActions);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindToInteractionComponent(UFVInteractionComponent* Component);
	void UnbindCurrentComponent();

	// DYNAMIC delegate handler — must be UFUNCTION
	UFUNCTION()
	void HandleFocusChanged(
		UFVInteractableComponent* FocusedInteractable, 
		const TArray<FFVInteractionActionDisplay>& AvailableActions);

	UFUNCTION()
	void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);

	TWeakObjectPtr<UFVInteractionComponent> BoundInteractionComponent;
};
