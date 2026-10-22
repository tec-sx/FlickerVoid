#include "Movement/FVMovementSetTransition.h"

#include "MoverComponent.h"
#include "Movement/FVMovementSetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMovementSetTransition)

FTransitionEvalResult UFVMovementSetTransition::Evaluate_Implementation(const FSimulationTickParams& Params) const
{
	PassedTransition.Reset();

	const UMoverComponent* Mover = GetTypedOuter<UMoverComponent>();
	const UFVMovementSetComponent* MovementSets = Mover ? UFVMovementSetComponent::Find(Mover->GetOwner()) : nullptr;
	if (!MovementSets)
	{
		return FTransitionEvalResult();
	}

	const FName CurrentMode = Params.StartState.SyncState.MovementMode;
	for (const FFVInstalledMovementSet& Installed : MovementSets->GetInstalledSets())
	{
		for (UBaseMovementModeTransition* Transition : Installed.Transitions)
		{
			const FTransitionEvalResult Result = Transition->Evaluate(Params);
			if (!Result.NextMode.IsNone() && (Result.NextMode != CurrentMode || Transition->bAllowModeReentry))
			{
				PassedTransition = Transition;
				return Result;
			}
		}
	}

	return FTransitionEvalResult();
}

void UFVMovementSetTransition::Trigger_Implementation(const FSimulationTickParams& Params)
{
	if (UBaseMovementModeTransition* Transition = PassedTransition.Get())
	{
		Transition->Trigger(Params);
	}
}
