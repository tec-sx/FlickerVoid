#include "FactDB/Flow/FVFlowNode_ChangeFactValue.h"

#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_ChangeFactValue)

UFVFlowNode_ChangeFactValue::UFVFlowNode_ChangeFactValue()
{
	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(TEXT("Out"))};
}

void UFVFlowNode_ChangeFactValue::ExecuteInput(const FName& PinName)
{
	if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem& Subsystem = UFVFactSubsystem::Get(World);
		Subsystem.ChangeFactValue(Fact, Value, ChangeType);
	}
	else
	{
		LogError(TEXT("No valid world"));
	}

	// Is it okay to trigger output in error case?
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFVFlowNode_ChangeFactValue::GetNodeDescription() const
{
	if (Fact.IsValid() == false)
	{
		return "None";
	}

	FStringFormatOrderedArguments Args;
	Args.Add(Fact.ToString());
	Args.Add(UEnum::GetDisplayValueAsText(ChangeType).ToString());
	Args.Add(Value);

	return FString::Format(TEXT("{0}: {1} {2}"), Args);
}
#endif
