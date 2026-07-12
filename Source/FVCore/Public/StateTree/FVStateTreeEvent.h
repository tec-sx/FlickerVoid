#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType, meta = (DisplayName = "State Tree Event"))
enum class EFVStateTreeEvent : uint8
{
	Unknown    UMETA(DisplayName = "Unknown"),
	Idle       UMETA(DisplayName = "Idle"),
	Patrol     UMETA(DisplayName = "Patrol"),
	Notice     UMETA(DisplayName = "Notice"),
	Focus      UMETA(DisplayName = "Focus"),
	Act        UMETA(DisplayName = "Act"),
};