#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/FVInteractionTypes.h"
#include "FVInteractionComponent.generated.h"

class UFVInteractableComponent;

//~=============================================================================
// Player-side interaction manager.
//
// Responsibilities:
//   - Sphere sweep each tick to find the best interactable in focus
//   - Broadcast focus changes to the UI
//   - Route input tag calls (E press, F hold, etc.) to the focused interactable
//~=============================================================================

UCLASS(Blueprintable, ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, 
		ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction) override;

	//~=========================================================================
	// Configuration
	//~=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	float DetectionRadius = 350.f;

	// Cosine of the half-angle of the cone in which an interactable can be focused
	// (0 = 90°, 0.5 = ~60°, 0.707 = 45°). Interactables outside this cone are ignored.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ClampMin = "-1", ClampMax = "1"))
	float FocusConeCosine = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ClampMin = "0"))
	float FocusUpdateInterval = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	TArray<TEnumAsByte<EObjectTypeQuery>> DetectionObjectTypes;

	//~=========================================================================
	// Input → Action routing (called from the input component, fully tag-based)
	//~=========================================================================

	// Route an input tag to the currently focused interactable.
	// Returns the result so the caller can give feedback (e.g. flash "requirement not met").
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	EFVInteractionResult RequestInteraction(const FGameplayTag& InputTag);

	// Cancel whatever the focused interactable is currently doing
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelCurrentInteraction();

	//~=========================================================================
	// State Accessors
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UFVInteractableComponent* GetFocusedInteractable() const { return FocusedInteractable.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasFocus() const { return FocusedInteractable.IsValid(); }

	//~=========================================================================
	// UI Events
	//~=========================================================================

	// Fires whenever focus changes (new interactable, or focus lost).
	// Bind this in your HUD to update the interaction prompt overlay.
	// Actions array is empty when focus is lost.
	UPROPERTY(BlueprintAssignable, Category = "Interaction|UI")
	FOnInteractableFocusChanged OnFocusChanged;

private:
	void UpdateFocus();
	UFVInteractableComponent* FindBestInteractable() const;
	void SetFocusedInteractable(UFVInteractableComponent* NewInteractable);
	void BroadcastFocusState() const;

	TWeakObjectPtr<UFVInteractableComponent> FocusedInteractable;
	float TimeSinceLastUpdate = 0.f;
};
