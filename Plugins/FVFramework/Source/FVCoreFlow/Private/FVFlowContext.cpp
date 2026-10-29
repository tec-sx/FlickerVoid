#include "FVFlowContext.h"

#include "Conditions/FVConditionLibrary.h"
#include "Nodes/FlowNodeBase.h"

FFVConditionContext FVFlow::MakeContext(const UFlowNodeBase& Node)
{
	AActor* FlowOwner = Node.TryGetRootFlowActorOwner();
	return UFVConditionLibrary::MakePlayerContext(Node.GetWorld(), FlowOwner);
}
