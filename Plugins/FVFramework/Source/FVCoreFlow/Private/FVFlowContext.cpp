#include "FVFlowContext.h"

#include "Kismet/GameplayStatics.h"
#include "Nodes/FlowNodeBase.h"

FFVConditionContext FVFlow::MakeContext(const UFlowNodeBase& Node)
{
	FFVConditionContext Context;
	Context.WorldContext = Node.GetWorld();
	Context.Instigator = UGameplayStatics::GetPlayerPawn(Context.WorldContext, 0);
	Context.Target = Node.TryGetRootFlowActorOwner();
	return Context;
}
