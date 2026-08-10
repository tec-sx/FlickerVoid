#include "FactDB/Flow/FVFlowNode_OnFactChanged.h"

#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_OnFactChanged)

UFVFlowNode_OnFactChanged::UFVFlowNode_OnFactChanged()
	: WantedValue(0),
	  SuccessLimit(1),
	  SuccessCount(0)
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Condition;
#endif

	InputPins = {FFlowPin(TEXT("Start")), FFlowPin(TEXT("Stop"))};
	OutputPins = {FFlowPin(TEXT("Success")), FFlowPin(TEXT("Completed")), FFlowPin(TEXT("Stopped"))};
}

void UFVFlowNode_OnFactChanged::ExecuteInput(const FName& PinName)
{
	if (Fact.IsValid())
	{
		if (PinName == TEXT("Start"))
		{
			StartObserving();
		}
		else if (PinName == TEXT("Stop"))
		{
			TriggerOutput(TEXT("Stopped"), true);
		}
	}
	else
	{
		LogError("No fact tag");
	}
}

void UFVFlowNode_OnFactChanged::OnLoad_Implementation()
{
	if (Fact.IsValid())
	{
		StartObserving();
	}
}

void UFVFlowNode_OnFactChanged::StartObserving()
{
	if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		FactSubsystem.GetOnFactValueChangedDelegate(Fact).AddUObject(this, &ThisClass::OnEventReceived);
	}
	else
	{
		LogError(TEXT("No valid world"));
		//TODO: What next?
	}
}

void UFVFlowNode_OnFactChanged::StopObserving()
{
	if (UWorld* World = GetWorld())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		FactSubsystem.GetOnFactValueChangedDelegate(Fact).RemoveAll(this);
	}
}

void UFVFlowNode_OnFactChanged::OnEventReceived(int32 CurrentValue)
{
	if (bWaitForExactValue && CurrentValue != WantedValue)
	{
		return;
	}

	TriggerOutput(TEXT("Success"));
	SuccessCount++;
	
	if (SuccessLimit > 0 && SuccessCount == SuccessLimit)
	{
		TriggerOutput(TEXT("Completed"), true);
	}
}

void UFVFlowNode_OnFactChanged::Cleanup()
{
	StopObserving();
	SuccessCount = 0;
	Super::Cleanup();
}

#if WITH_EDITOR
FString UFVFlowNode_OnFactChanged::GetNodeDescription() const
{
	FStringFormatOrderedArguments Args;
	Args.Add( Fact.IsValid() ? Fact.ToString() : "None" );
	Args.Add( bWaitForExactValue ? FString::FromInt( WantedValue ) : "Any" );
	
	return FString::Format( TEXT("{0}\nWanted value:{1}"), Args );
}
#endif
