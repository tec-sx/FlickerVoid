#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Tasks/StateTreeTaskBlueprintBase.h"
#include "FVInteractionStateTreeTaskBase.generated.h"

class UFVInteractableComponent;

//~=============================================================================
// Base class for all interaction State Tree tasks.
//
// Subclass in AngelScript (or C++) to implement an action's execution logic.
// Configuration lives as UPROPERTY fields on the task subclass — the State Tree
// editor exposes them per-node, replacing the old HandlerConfig subclass pattern.
//
// Lifecycle:
//   ReceiveEnterState  — start the action; for sync tasks call CompleteTask() here.
//   ReceiveTick        — optional; driven by UStateTreeComponent each frame.
//                        Default C++ implementation polls IsActiveTaskDone() so
//                        async tasks complete without overriding Tick.
//   ReceiveExitState   — cleanup on cancel or natural exit.
//
// For async tasks (dialogue, mini-games):
//   Do NOT override ReceiveTick (or call Super if you do). The base polls
//   UFVInteractableComponent until CompleteTask() is called from your callback.
//
// For tasks needing per-frame work (e.g. advancing a lockpick slider):
//   Override ReceiveTick and call Super.ReceiveTick(OwnerActor, DeltaTime) first
//   so completion is still detected on the same frame.
//~=============================================================================

UCLASS(Abstract, Blueprintable, BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVInteractionStateTreeTaskBase : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	//~=========================================================================
	// Context helpers — pass OwnerActor from ReceiveEnterState / ReceiveTick
	//~=========================================================================

	// The interactable component on the actor running this State Tree.
	UFUNCTION(BlueprintPure, Category = "Interaction|Task")
	static UFVInteractableComponent* GetInteractable(AActor* OwnerActor);

	// The actor (player) that triggered this interaction.
	UFUNCTION(BlueprintPure, Category = "Interaction|Task")
	static AActor* GetInstigator(AActor* OwnerActor);

	// The action tag that was triggered (e.g. Interaction.Action.PickUp).
	UFUNCTION(BlueprintPure, Category = "Interaction|Task")
	static FGameplayTag GetActionTag(AActor* OwnerActor);

	// World position of the interaction point (for IK warping, VFX, etc.).
	UFUNCTION(BlueprintPure, Category = "Interaction|Task")
	static FVector GetInteractionPoint(AActor* OwnerActor);

	//~=========================================================================
	// Completion — call when async work finishes
	//~=========================================================================

	// Signal that this task is done. The State Tree picks it up on the next Tick.
	// Sync tasks: call inside ReceiveEnterState.
	// Async tasks: call from your dialogue-ended / mini-game-finished callback.
	UFUNCTION(BlueprintCallable, Category = "Interaction|Task")
	static void CompleteTask(AActor* OwnerActor, bool bSuccess);

protected:
	// Default ReceiveTick polls UFVInteractableComponent::IsActiveTaskDone().
	// Override for per-frame logic, but call Super first to preserve this check.
	virtual EStateTreeRunStatus ReceiveTick_Implementation(
		AActor* OwnerActor, float DeltaTime) override;
};
