#pragma once

#include "CoreMinimal.h"
#include "Interaction/FVInteractionTypes.h"
#include "Interaction/FVInteractionAction.h"
#include "FVInteractionActionHandler.generated.h"

//~=============================================================================
// Abstract handler — subclass in C++, Blueprint, or AngelScript
//
// Lifecycle (mirrors movement handler pattern):
//   OnInitialize  — called once, receives config
//   Execute       — called when the player triggers the action; starts execution
//   Cancel        — called if execution is interrupted
//   OnCompleted   — fires when async work finishes (mini-game, cutscene, etc.)
//
// Sync handlers: call CompleteExecution() inside Execute().
// Async handlers: call CompleteExecution() from the mini-game callback.
//~=============================================================================

UCLASS(Abstract, Blueprintable, BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVInteractionActionHandler : public UObject
{
	GENERATED_BODY()

public:
	UFVInteractionActionHandler();

	//~=========================================================================
	// Lifecycle — override in script/Blueprint
	//~=========================================================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Handler")
	void OnInitialize(UFVInteractionHandlerConfig* Config);

	// Start executing the action. For sync actions call CompleteExecution() here.
	// For async (mini-games) open the UI and wait for user input.
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Handler")
	void OnExecute(const FFVInteractionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Handler")
	void OnCancel(const FFVInteractionContext& Context);

	//~=========================================================================
	// C++ interface (called by UFVInteractableComponent)
	//~=========================================================================

	void Initialize(UFVInteractionHandlerConfig* Config);
	EFVInteractionResult Execute(const FFVInteractionContext& Context);
	void Cancel(const FFVInteractionContext& Context);

	//~=========================================================================
	// State
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction|Handler")
	EFVInteractionStatus GetStatus() const { return Status; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Handler")
	bool IsExecuting() const { return Status == EFVInteractionStatus::Executing; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Handler")
	const FFVInteractionContext& GetActiveContext() const { return ActiveContext; }

	//~=========================================================================
	// Call this from Blueprint/AngelScript when async execution finishes
	//~=========================================================================

	UFUNCTION(BlueprintCallable, Category = "Interaction|Handler")
	void CompleteExecution(bool bSuccess);

	//~=========================================================================
	// Events — bind to these for post-action consequences (UI, stats, quests)
	//~=========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Handler")
	FOnInteractionCompleted OnCompleted;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Handler")
	FFVInteractionContext ActiveContext;

private:
	EFVInteractionStatus Status;
	bool bIsInitialized;
};
