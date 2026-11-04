#pragma once

#include "CoreMinimal.h"

class UActorComponent;
class UClass;
struct FInteractionCompileContext;

namespace InteractionBlueprintComponentUtils
{
	/**
	 * Collects the component templates an actor class spawns with: native and AngelScript default components (from the class defaults),
	 * and the construction script components of every Blueprint in its chain, with child Blueprint overrides applied.
	 */
	void CollectComponentTemplates(const UClass* ActorClass, TArray<const UActorComponent*>& OutTemplates);

	/** Runs every registered interaction rule on the components. */
	void RunRules(const FInteractionCompileContext& Context);
}
