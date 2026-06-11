#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StateTreeTypes.h"
#include "Interaction/FVInteractionTypes.h"
#include "Interaction/FVInteractionAction.h"

#include "FVInteractableComponent.generated.h"

class UStateTreeComponent;

//~=============================================================================
// Placed on any world actor (item, door, machine, NPC) to make it interactable.
//
// Holds up to 4 actions. Each action maps to one of four input slots and
// references a State Tree asset that defines the execution flow.
//
// The component manages one UStateTreeComponent on the owner actor. When an
// action is triggered the relevant State Tree runs; UFVInteractionStateTreeTaskBase
// subclass tasks access context (instigator, action tag, etc.) via the helpers
// on this component.
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
	// Active context — read by UFVInteractionStateTreeTaskBase helpers
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction|ActiveContext")
	AActor* GetActiveInstigator() const { return ActiveInstigator.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction|ActiveContext")
	FGameplayTag GetActiveActionTag() const { return ActiveActionTag; }

	UFUNCTION(BlueprintPure, Category = "Interaction|ActiveContext")
	FVector GetActiveInteractionPoint() const { return ActiveInteractionPoint; }

	// True after CompleteActiveTask() was called — polled by the task base each Tick.
	UFUNCTION(BlueprintPure, Category = "Interaction|ActiveContext")
	bool IsActiveTaskDone() const { return bActiveTaskDone; }

	UFUNCTION(BlueprintPure, Category = "Interaction|ActiveContext")
	bool DidActiveTaskSucceed() const { return bActiveTaskSucceeded; }

	// Called by UFVInteractionStateTreeTaskBase::CompleteTask — do not call directly
	// from gameplay code; call CompleteTask(OwnerActor, bSuccess) on the task base instead.
	UFUNCTION(BlueprintCallable, Category = "Interaction|ActiveContext")
	void CompleteActiveTask(bool bSuccess);

	//~=========================================================================
	// Called by UFVInteractionComponent
	//~=========================================================================

	// Build display data for the UI — includes availability per action.
	TArray<FFVInteractionActionDisplay> GetActionDisplayData(AActor* Instigator) const;

	// Attempt to execute the action bound to InputTag. Returns result.
	EFVInteractionResult TryExecuteAction(const FGameplayTag& InputTag, AActor* Instigator);

	// Cancel the currently running State Tree (if any).
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
	void CreateStateTreeComponent();

	UFUNCTION()
	void OnStateTreeStopped(UStateTreeComponent* Comp, EStateTreeRunStatus RunStatus);

	UPROPERTY(Transient)
	TObjectPtr<UStateTreeComponent> InteractionStateTreeComp;

	// Context set before each tree run — read by tasks via GetActive* accessors
	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveInstigator;

	UPROPERTY(Transient)
	FGameplayTag ActiveActionTag;

	UPROPERTY(Transient)
	FVector ActiveInteractionPoint = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bActiveTaskDone = false;

	UPROPERTY(Transient)
	bool bActiveTaskSucceeded = false;

	// Cached action tag for the completion callback (ActiveActionTag may be cleared before broadcast)
	FGameplayTag CompletingActionTag;

	bool bIsInFocus = false;
};
