#include "FactDB/Flow/FVFlowNode_WaitForFactCondition.h"

#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_WaitForFactCondition)

UFVFlowNode_WaitForFactCondition::UFVFlowNode_WaitForFactCondition()
{
	InputPins = {FFlowPin(TEXT("Start")), FFlowPin(TEXT("Stop"))};
	OutputPins = {FFlowPin(TEXT("Completed")), FFlowPin(TEXT("Stopped"))};

#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Condition;
	Category = TEXT("Fact");
#endif
}

void UFVFlowNode_WaitForFactCondition::ExecuteInput(const FName& PinName)
{
	if (PinName == TEXT("Stop"))
	{
		StopWaiting();
		TriggerOutput(TEXT("Stopped"), true);
		return;
	}

	StartWaiting();
}

void UFVFlowNode_WaitForFactCondition::OnLoad_Implementation()
{
	if (bIsWaiting)
	{
		bIsWaiting = false;
		StartWaiting();
	}
}

void UFVFlowNode_WaitForFactCondition::StartWaiting()
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LogError(TEXT("No valid world"));
		return;
	}

	if (Conditions.Conditions.IsEmpty())
	{
		LogError(TEXT("No conditions to wait for"));
		TriggerOutput(TEXT("Completed"), true);
		return;
	}

	UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);

	// Check before subscribing: the condition may already hold, and waiting for a
	// further change would deadlock the graph.
	if (FactSubsystem.CheckFactConditions(Conditions))
	{
		TriggerOutput(TEXT("Completed"), true);
		return;
	}

	if (bIsWaiting)
	{
		return;
	}

	bIsWaiting = true;
	for (const FFVFactCondition& Condition : Conditions.Conditions)
	{
		if (Condition.Tag.IsValid())
		{
			FactSubsystem.GetOnFactValueChangedDelegate(Condition.Tag)
						 .AddUObject(this, &ThisClass::HandleWatchedFactChanged);
		}
	}
}

void UFVFlowNode_WaitForFactCondition::StopWaiting()
{
	if (!bIsWaiting)
	{
		return;
	}

	bIsWaiting = false;

	if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		for (const FFVFactCondition& Condition : Conditions.Conditions)
		{
			if (Condition.Tag.IsValid())
			{
				FactSubsystem.GetOnFactValueChangedDelegate(Condition.Tag).RemoveAll(this);
			}
		}
	}
}

void UFVFlowNode_WaitForFactCondition::HandleWatchedFactChanged(int32 NewValue)
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (UFVFactSubsystem::Get(World).CheckFactConditions(Conditions))
	{
		StopWaiting();
		TriggerOutput(TEXT("Completed"), true);
	}
}

void UFVFlowNode_WaitForFactCondition::Cleanup()
{
	StopWaiting();
	Super::Cleanup();
}

#if WITH_EDITOR
EDataValidationResult UFVFlowNode_WaitForFactCondition::ValidateNode()
{
	if (Conditions.IsValid())
	{
		return EDataValidationResult::Valid;
	}

	ValidationLog.Error<UFlowNode>(TEXT("Missing or invalid conditions"), this);
	return EDataValidationResult::Invalid;
}

FString UFVFlowNode_WaitForFactCondition::GetNodeDescription() const
{
	return Conditions.ToString();
}
#endif
