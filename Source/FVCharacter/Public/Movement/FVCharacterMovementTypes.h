#pragma once

#include "FVCharacterMovementTypes.generated.h"

UENUM(BlueprintType)
enum class EFVRotationMode : uint8
{
	OrientToMovement,
	Strafe,
	Aim
};

UENUM(BlueprintType)
enum class EFVMovementHipDirection : uint8
{
	Forward,
	Backward,
	Left_HipLeft,
	Left_HipRight,
	Right_HipLeft,
	Right_HipRight
};
