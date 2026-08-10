#include "FactDB/Flow/FVFlowNode_OnFactDefined.h"
#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_OnFactDefined)

UFVFlowNode_OnFactDefined::UFVFlowNode_OnFactDefined()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Condition;
#endif
}

void UFVFlowNode_OnFactDefined::ExecuteInput(const FName& PinName)
{
	if (PinName == DefaultInputPin.PinName && Fact.IsValid())
	{
		StartObserving();
	}
	else
	{
		LogError("No fact tag");
	}
}

void UFVFlowNode_OnFactDefined::OnLoad_Implementation()
{
	if (Fact.IsValid())
	{
		StartObserving();
	}
}

void UFVFlowNode_OnFactDefined::StartObserving()
{
	if (UWorld* World = GetWorld())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		FactSubsystem.GetOnFactBecameDefinedDelegate(Fact).AddUObject(this, &ThisClass::OnEventReceived);
	}
	else
	{
		LogError(TEXT("No valid world"));
		//TODO: What next?
	}
}

void UFVFlowNode_OnFactDefined::StopObserving()
{
	if (UWorld* World = GetWorld())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		FactSubsystem.GetOnFactBecameDefinedDelegate(Fact).RemoveAll(this);
	}
}

void UFVFlowNode_OnFactDefined::OnEventReceived(int32 CurrentValue)
{
	if (!ArePredicateAddOnsSatisfied())
	{
		return;
	}

	TriggerFirstOutput(true);
}

void UFVFlowNode_OnFactDefined::Cleanup()
{
	StopObserving();
	Super::Cleanup();
}

#if WITH_EDITOR
FString UFVFlowNode_OnFactDefined::GetNodeDescription() const
{
	return Fact.IsValid() ? Fact.ToString() : "None";
}
#endif
