#pragma once

#include "CoreMinimal.h"

class UActorComponent;
class UBlueprint;

namespace InteractionBlueprintComponentUtils
{
	bool HasComponentOfClass(const UBlueprint& Blueprint, const TSubclassOf<UActorComponent>& ComponentClass);
}
