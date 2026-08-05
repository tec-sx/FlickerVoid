#include "Flow/Nodes/FVFlowNode_OnTriggerEvent.h"

#include "FVCoreTags.h"
#include "Flow/Components/FVFlowTriggerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_OnTriggerEvent)

UFVFlowNode_OnTriggerEvent::UFVFlowNode_OnTriggerEvent()
	: bReactOnOverlapping(false)
{
	OverlappedActorTags = FGameplayTagContainer(FVCoreTags::Player_Pawn);
}

void UFVFlowNode_OnTriggerEvent::ExecuteInput(const FName& PinName)
{
	if (OverlappedActorTags.IsValid())
	{
		Super::ExecuteInput(PinName);
	}
	else
	{
		LogError("Overlapped Actor Tag is missing!");
	}
}

void UFVFlowNode_OnTriggerEvent::ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	if (!RegisteredActors.Contains(Actor))
	{
		if (UFVFlowTriggerComponent* TriggerComponent = Cast<UFVFlowTriggerComponent>(Component))
		{
			RegisteredActors.Emplace(Actor, TriggerComponent);
			TriggerComponent->OnTriggerEvent.AddDynamic(this, &UFVFlowNode_OnTriggerEvent::OnTriggerEvent);
		}
	}
}

void UFVFlowNode_OnTriggerEvent::ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	UFVFlowTriggerComponent* TriggerComponent = Cast<UFVFlowTriggerComponent>(Component);
	ensureAlways(TriggerComponent);

	TriggerComponent->OnTriggerEvent.RemoveAll(this);
}

void UFVFlowNode_OnTriggerEvent::OnTriggerEvent(const bool bOverlapping, UFlowComponent* OtherFlowComponent)
{
	if (bReactOnOverlapping == bOverlapping && OtherFlowComponent->IdentityTags.HasAnyExact(OverlappedActorTags))
	{
		OnEventReceived();
	}
}

#if WITH_EDITOR
EDataValidationResult UFVFlowNode_OnTriggerEvent::ValidateNode()
{
	if (IdentityTags.IsEmpty() || OverlappedActorTags.IsEmpty())
	{
		if (IdentityTags.IsEmpty())
		{
			ValidationLog.Error<UFlowNode>(*UFlowNode::MissingIdentityTag, this);
		}
		if (OverlappedActorTags.IsEmpty())
		{
			ValidationLog.Error<UFlowNode>(TEXT("Overlapped Actor Tag is missing!"), this);
		}
		
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif