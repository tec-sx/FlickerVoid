#include "FVFlowContext.h"

#include "Conditions/FVConditionLibrary.h"
#include "Nodes/FlowNodeBase.h"

FFVConditionContext FVFlow::MakeContext(const UFlowNodeBase& Node)
{
	return UFVConditionLibrary::MakePlayerContext(Node.GetWorld(), Node.TryGetRootFlowActorOwner());
}
