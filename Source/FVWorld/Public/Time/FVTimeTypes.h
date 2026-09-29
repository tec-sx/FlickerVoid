#pragma once

#include "CoreMinimal.h"
#include "FVTimeTypes.generated.h"

/** Value stored in Fact.World.Time.Phase. */
UENUM(BlueprintType)
enum class EFVTimeOfDay : uint8
{
	Night = 0,
	Morning = 25,
	Afternoon = 50,
	Evening = 75
};
