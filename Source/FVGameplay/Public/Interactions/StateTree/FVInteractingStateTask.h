#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Interactions/FVInteractionTypes.h"

#include "FVInteractingStateTask.generated.h"

class UFVInteractionOfferComponent;

//~=============================================================================
// Player-side "Interacting" state.
//
// Entered while the engaged target's own State Tree runs the action. This task
// owns only the player's side of the handshake: it tags the player as
// interacting (suppressing movement and further interaction input) and
// finishes as soon as the target reports the interaction is over.
//~=============================================================================

UCLASS(Blueprintable, DisplayName = "FV Interacting")
class FLICKERVOIDGAMEPLAY_API UFVInteractingStateTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	/** The player actor. Bind to the tree's context actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> InteractionOwner;

	/** Stops the owner's movement input while the interaction runs. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bSuppressMovement = true;

protected:
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) override;

private:
	void ApplyInteractingState(bool bInteracting);

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractionOfferComponent> Offers;
};
