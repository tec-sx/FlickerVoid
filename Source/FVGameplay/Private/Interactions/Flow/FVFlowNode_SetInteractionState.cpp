#include "Flow/Nodes/FVFlowNode_SetInteractionState.h"

#include "FlowComponent.h"
#include "FlowSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_SetInteractionState)

UFVFlowNode_SetInteractionState::UFVFlowNode_SetInteractionState()
{
#if WITH_EDITOR
	Category = TEXT("Actor");
#endif

	InputPins = {
		FFlowPin(TEXT("Enable")), 
		FFlowPin(TEXT("Disable"))
	};
}

void UFVFlowNode_SetInteractionState::ExecuteInput(const FName& PinName)
{
	TSet<TWeakObjectPtr<UFlowComponent>> Components = 
		GetFlowSubsystem()->GetComponents<UFlowComponent>(IdentityTags, EGameplayContainerMatchType::Any);
	
	for (const TWeakObjectPtr<UFlowComponent>& FoundComponent : Components)
	{
		TArray<UInteractionComponent*> FoundInteractions;
		FoundComponent->GetOwner()->GetComponents<UInteractionComponent>(FoundInteractions);
		if (FoundInteractions.Num() > 0)
		{
			if (PinName == TEXT("Enable"))
			{
				FoundInteractions[0]->Enable();
			}
			else
			{
				FoundInteractions[0]->Disable();
			}
		}
	}

	TriggerFirstOutput(true);
}

FString UFVFlowNode_SetInteractionState::GetNodeDescription() const
{
	return Super::GetNodeDescription();
}

EDataValidationResult UFVFlowNode_SetInteractionState::ValidateNode()
{
	return Super::ValidateNode();
}
