#include "Interactions/Flow/FVFlowNode_OnInteract.h"

#include "FlowComponent.h"
#include "Interactions/FVInteractionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_OnInteract)

void UFVFlowNode_OnInteract::ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
	if (!ObservedInteractions.Contains(Actor))
	{
		TArray<UFVInteractionComponent*> FoundInteractions;
		Actor->GetComponents<UFVInteractionComponent>(FoundInteractions);
	
		if (FoundInteractions.Num() > 0)
		{
			RegisteredActors.Emplace(Actor, Component);
		
			ObservedInteractions.Emplace(Actor, FoundInteractions[0]);
			FoundInteractions[0]->OnInteracted.AddDynamic(this, &UFVFlowNode_OnInteract::OnEventReceived);
		}
	}
}

void UFVFlowNode_OnInteract::ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component)
{
		ensureAlways(ObservedInteractions.Contains(Component->GetOwner()));
		const TWeakObjectPtr<UFVInteractionComponent> InteractionComponent = ObservedInteractions[Component->GetOwner()];
	
		InteractionComponent->OnInteracted.RemoveAll(this);
}

void UFVFlowNode_OnInteract::Cleanup()
{
	Super::Cleanup();

	for (const TPair<TWeakObjectPtr<AActor>, TWeakObjectPtr<UFVInteractionComponent>>& Interaction : ObservedInteractions)
	{
		Interaction.Value->OnInteracted.RemoveAll(this);
	}
	ObservedInteractions.Empty();
}
