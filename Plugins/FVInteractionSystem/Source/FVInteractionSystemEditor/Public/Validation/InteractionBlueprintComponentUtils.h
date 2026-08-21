#pragma once

#include "CoreMinimal.h"

class UActorComponent;
class UBlueprint;

namespace InteractionBlueprintComponentUtils
{
	bool HasComponentOfClass(const UBlueprint& Blueprint, const TSubclassOf<UActorComponent>& ComponentClass);

	/** Collects the component templates authored on the blueprint's construction script. */
	void GetComponentTemplatesOfClass(const UBlueprint& Blueprint, const TSubclassOf<UActorComponent>& ComponentClass, TArray<const UActorComponent*>& OutTemplates);
}
