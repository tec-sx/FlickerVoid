#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"

class UFlowNodeBase;

namespace FVFlow
{
	/** Instigator = first local player pawn, Target = actor owning the root flow. */
	FVCOREFLOW_API FFVConditionContext MakeContext(const UFlowNodeBase& Node);
}
