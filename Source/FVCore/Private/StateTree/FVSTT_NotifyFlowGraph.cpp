// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVSTT_NotifyFlowGraph.h"

#include "FlowComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FFVSTT_NotifyFlowGraph::EnterState(FStateTreeExecutionContext& Context,
                                                       const FStateTreeTransitionResult& Transition) const
{
	Super::EnterState(Context, Transition);
	
	const FNotifyFlowGraphInstanceData& Data = Context.GetInstanceData(*this);
	
	if (!Data.Actor || !Data.NotifyTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (Data.Actor->HasActorBegunPlay())
	{
		UFlowComponent* FlowComponent = Data.Actor->FindComponentByClass<UFlowComponent>();
	
		if (!FlowComponent)
		{
			return EStateTreeRunStatus::Failed;
		}
		
		FlowComponent->NotifyGraph(Data.NotifyTag);
		
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}
}
