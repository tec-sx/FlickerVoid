#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interaction/FVInteractionTypes.h"
#include "Interaction/FVInteractionAction.h"

#include "FVInteractableComponent.generated.h"

class UFVInteractionActionHandler;

//~=============================================================================
// Placed on any world actor (item, door, machine, NPC) to make it interactable.
//
// Holds up to 4 actions. Each action maps to one of four input slots.
// The component instantiates handlers at BeginPlay and keeps them alive for
// the actor's lifetime, so handlers can be stateful.
//~=============================================================================

UCLASS(Blueprintable, ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractableComponent();

	//~=========================================================================
	// Configuration (set per-actor in the editor)
	//~=========================================================================

	// Up to 4 actions. Order determines fallback priority when two actions share an InputTag.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
		meta = (TitleProperty = "DisplayName"))
	TArray<FFVInteractionAction> Actions;

	// Detection range at which focus begins (outer ring). Trace only checks within this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	float FocusRadius = 300.f;

	// Within this radius actions can actually be triggered.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	float InteractionRadius = 200.f;

	// Tags on this interactable that describe it (Interactable.Door.Locked, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTagContainer InteractableTags;

	//~=========================================================================
	// Runtime State
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInFocus() const { return bIsInFocus; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsBeingInteracted() const;

	//~=========================================================================
	// Called by UFVInteractionComponent
	//~=========================================================================

	// Build display data for the UI — includes availability per action.
	TArray<FFVInteractionActionDisplay> GetActionDisplayData(AActor* Instigator) const;

	// Attempt to execute the action bound to InputTag. Returns result.
	EFVInteractionResult TryExecuteAction(const FGameplayTag& InputTag, AActor* Instigator);

	// Cancel the currently running handler (if any and if cancellable).
	void CancelActiveInteraction();

	void SetFocused(bool bFocused);

	//~=========================================================================
	// Events — bind in Blueprint/AngelScript for visual feedback
	//~=========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionCompleted OnAnyActionCompleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, bool, bFocused);
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnFocusChanged OnFocusChanged;

	//~=========================================================================
	// UActorComponent Interface
	//~=========================================================================

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeHandlers();

	UFUNCTION()
	void HandleActionCompleted(const FFVInteractionContext& Context, EFVInteractionStatus Status, bool bSuccess);

	// Instantiated handlers — one per action slot, alive for actor lifetime
	UPROPERTY()
	TArray<TObjectPtr<UFVInteractionActionHandler>> HandlerInstances;

	UPROPERTY()
	TObjectPtr<UFVInteractionActionHandler> ActiveHandler;

	bool bIsInFocus = false;
};
