#include "Flow/Nodes/FVFlowNode_NotifyStateTree.h"

#include "FlowAsset.h"
#include "FlowComponent.h"
#include "Components/StateTreeComponent.h"

UFVFlowNode_NotifyStateTree::UFVFlowNode_NotifyStateTree()
{
	InputPins  = { FFlowPin(TEXT("In")) };
	OutputPins = { FFlowPin(TEXT("Out")) };
}

void UFVFlowNode_NotifyStateTree::ExecuteInput(const FName& PinName)
{
	if (const UFlowComponent* OwnerComp = Cast<UFlowComponent>(GetFlowAsset()->GetOwner()))
	{
		if (const AActor* Owner = OwnerComp->GetOwner())
		{
			if (UStateTreeComponent* Tree = Owner->FindComponentByClass<UStateTreeComponent>())
			{
				Tree->SendStateTreeEvent(EventTag, FConstStructView(), NAME_None);
			}
		}
		TriggerOutput(TEXT("Out"), true);
	}
}
