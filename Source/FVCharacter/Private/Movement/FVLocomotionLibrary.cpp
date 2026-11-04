#include "Movement/FVLocomotionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLocomotionLibrary)

bool UFVLocomotionLibrary::GetLocomotionInputs(const FMoverTickStartData& StartState, FFVLocomotionInputs& OutInputs)
{
	if (const FFVLocomotionInputs* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FFVLocomotionInputs>())
	{
		OutInputs = *Inputs;
		return true;
	}

	return false;
}

bool UFVLocomotionLibrary::GetLocomotionInputsFromTick(const FSimulationTickParams& Params, FFVLocomotionInputs& OutInputs)
{
	return GetLocomotionInputs(Params.StartState, OutInputs);
}
