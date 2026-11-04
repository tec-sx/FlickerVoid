#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Movement/FVLocomotionInputs.h"
#include "MoverSimulationTypes.h"
#include "FVLocomotionLibrary.generated.h"

/** Reads FlickerVoid's locomotion inputs inside Mover movement modes and transitions, including AngelScript ones. */
UCLASS()
class FLICKERVOIDCHARACTER_API UFVLocomotionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Locomotion", meta = (ToolTip = "Locomotion inputs of this step, for a movement mode's Generate Move. False when the pawn sent none."))
	static bool GetLocomotionInputs(const FMoverTickStartData& StartState, FFVLocomotionInputs& OutInputs);

	UFUNCTION(BlueprintPure, Category = "Locomotion", meta = (ToolTip = "Locomotion inputs of this step, for a mode's Simulation Tick or a transition's Evaluate. False when the pawn sent none."))
	static bool GetLocomotionInputsFromTick(const FSimulationTickParams& Params, FFVLocomotionInputs& OutInputs);
};
