#pragma once

#include "CoreMinimal.h"
#include "MovementModeTransition.h"
#include "FVMovementSetTransition.generated.h"

/**
 * Global Mover transition the movement set component adds once, before Mover initializes. Checks the transitions of the
 * owner's movement sets in order, so sets added at runtime bring their transitions too: Mover only reads its own
 * global transitions on initialize.
 */
UCLASS(NotBlueprintable, HideDropdown)
class FLICKERVOIDCHARACTER_API UFVMovementSetTransition : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
	virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;

private:
	mutable TWeakObjectPtr<UBaseMovementModeTransition> PassedTransition;
};
