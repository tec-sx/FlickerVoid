#include "StateTree/FVSTT_TriggerFlowGraph.h"

#include "AITestsCommon.h"
#include "FlowComponent.h"
#include "StateTreeExecutionContext.h"
#include "Flow/Components/FVFlowTriggerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSTT_TriggerFlowGraph)

EStateTreeRunStatus FFVSTT_TriggerFlowGraph::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	Super::EnterState(Context, Transition);
	
	const FTriggerFlowGraphInstanceData& Data = Context.GetInstanceData(*this);

	if (!Data.Actor || !Data.OtherActor || Data.IdentityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	UFVFlowTriggerComponent* FlowTriggerComponent = Data.Actor->FindComponentByClass<UFVFlowTriggerComponent>();
	UFlowComponent* FlowComponent = Data.OtherActor->FindComponentByClass<UFlowComponent>();
	
	if (!FlowTriggerComponent || !FlowComponent)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (Data.Actor->HasActorBegunPlay())
	{
		FlowTriggerComponent->OnTriggerEvent.Broadcast(true, FlowComponent);
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		TWeakObjectPtr<UFVFlowTriggerComponent> FlowTriggerComponentWeakPtr(FlowTriggerComponent);
		TWeakObjectPtr<UFlowComponent> FlowComponentWeakPtr(FlowComponent);
		
		Context.GetWorld()->GetTimerManager().SetTimerForNextTick(
			[FlowTriggerComponentWeakPtr, FlowComponentWeakPtr]()
			{
				FlowTriggerComponentWeakPtr->OnTriggerEvent.Broadcast(true, FlowComponentWeakPtr.Get());
				return EStateTreeRunStatus::Succeeded;
			});
		
		return EStateTreeRunStatus::Running;
	}
}

void FFVSTT_TriggerFlowGraph::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FTriggerFlowGraphInstanceData& Data = Context.GetInstanceData(*this);
	
	UFVFlowTriggerComponent* FlowTriggerComponent = Data.Actor->FindComponentByClass<UFVFlowTriggerComponent>();
	UFlowComponent* FlowComponent = Data.OtherActor->FindComponentByClass<UFlowComponent>();
	
	if (FlowTriggerComponent && FlowComponent)
	{
		FlowTriggerComponent->OnTriggerEvent.Broadcast(false, FlowComponent);
	}
	
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
