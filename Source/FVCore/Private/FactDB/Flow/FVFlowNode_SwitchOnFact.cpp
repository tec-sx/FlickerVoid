#include "FactDB/Flow/FVFlowNode_SwitchOnFact.h"

#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_SwitchOnFact)

namespace
{
	const FName DefaultPinName(TEXT("Default"));
}

UFVFlowNode_SwitchOnFact::UFVFlowNode_SwitchOnFact()
{
	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(DefaultPinName)};
}

FName UFVFlowNode_SwitchOnFact::MakeValuePinName(int32 Value)
{
	return FName(*FString::FromInt(Value));
}

void UFVFlowNode_SwitchOnFact::ExecuteInput(const FName& PinName)
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LogError(TEXT("No valid world"));
		TriggerOutput(DefaultPinName, true);
		return;
	}

	const UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);

	int32 CurrentValue = 0;
	const bool bDefined = FactSubsystem.GetFactValueIfDefined(Fact, CurrentValue);

	if ((bDefined || !bUndefinedTakesDefault) && Values.Contains(CurrentValue))
	{
		TriggerOutput(MakeValuePinName(CurrentValue), true);
		return;
	}

	TriggerOutput(DefaultPinName, true);
}

#if WITH_EDITOR
TArray<FFlowPin> UFVFlowNode_SwitchOnFact::GetContextOutputs() const
{
	TArray<FFlowPin> Pins;
	Pins.Reserve(Values.Num() + 1);

	TSet<int32> SeenValues;
	for (const int32 Value : Values)
	{
		bool bAlreadySeen = false;
		SeenValues.Add(Value, &bAlreadySeen);
		if (!bAlreadySeen)
		{
			Pins.Emplace(MakeValuePinName(Value));
		}
	}

	Pins.Emplace(DefaultPinName);
	return Pins;
}

void UFVFlowNode_SwitchOnFact::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFVFlowNode_SwitchOnFact, Values))
	{
		OnReconstructionRequested.ExecuteIfBound();
	}
}

EDataValidationResult UFVFlowNode_SwitchOnFact::ValidateNode()
{
	EDataValidationResult Result = Super::ValidateNode();

	if (Values.IsEmpty())
	{
		ValidationLog.Warning<UFlowNode>(TEXT("No values configured, everything takes Default"), this);
	}

	return Result;
}

FString UFVFlowNode_SwitchOnFact::GetNodeDescription() const
{
	return Fact.IsValid() ? Fact.ToString() : TEXT("None");
}
#endif
