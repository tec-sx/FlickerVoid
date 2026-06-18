#pragma once

#include "CoreMinimal.h"
#include "FVInteractionTargetConfig.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StateTreeTypes.h"
#include "Interactions/FVInteractionTypes.h"

#include "FVInteractionTargetComponent.generated.h"

class UFVInteractionTargetConfig;
class UStateTreeComponent;

//~=============================================================================
// Placed on any world actor (item, door, machine, NPC) to make it interactable.
// The component manages one UStateTreeComponent on the owner actor. When an
// action is triggered the relevant State Tree runs; UFVInteractionStateTreeTaskBase
// subclass tasks access context (instigator, action tag, etc.) via the helpers
// on this component.
//~=============================================================================

UCLASS(Blueprintable, ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionTargetComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//~=========================================================================
	// Configuration
	//~=========================================================================
	
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UFVInteractionTargetConfig> Config;

	//~=========================================================================
	// Runtime State
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInFocus() const { return bIsInFocus; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractionInProgress() const;

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
	
	UFUNCTION(BlueprintCallable, Category = "Interaction|Actions")
	EFVInteractionResult TryExecuteAction(const FGameplayTag& ActionTag, AActor* InstigatorActor);
	
	void CancelActiveInteraction();
	void SetFocused(bool bFocused);
	float GetFocusRadius() const { return Config->FocusRadius; }
	
	UFUNCTION(BlueprintPure, Category = "Interaction|Actions")
	TArray<UFVInteractionAction*> GetAvailableActions() const { return Config->AvailableActions; }

	//~=========================================================================
	// Events — bind in Blueprint/AngelScript for visual feedback
	//~=========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionCompleted OnAnyActionCompleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, bool, bFocused);
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnFocusChanged OnFocusChanged;
	
private:
	UFUNCTION()
	void OnStateTreeStatusChanged(EStateTreeRunStatus RunStatus);

	UPROPERTY(Transient)
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

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
	
	UPROPERTY(Transient)
	bool bActiveActionIsSimple = false;
	
	// Cached action tag for the completion callback (ActiveActionTag may be cleared before broadcast)
	FGameplayTag CompletingActionTag;

	bool bIsInitialized = false;
	bool bIsInFocus = false;
};
