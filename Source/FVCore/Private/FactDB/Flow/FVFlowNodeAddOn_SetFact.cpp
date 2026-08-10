#include "FactDB/Flow/FVFlowNodeAddOn_SetFact.h"

#include "FlowSettings.h"
#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNodeAddOn_SetFact)

UFVFlowNodeAddOn_SetFact::UFVFlowNodeAddOn_SetFact()
{
#if WITH_EDITOR
	Category = TEXT("Fact");
#endif
}

void UFVFlowNodeAddOn_SetFact::ExecuteInput(const FName& PinName)
{
	if (!Fact.IsValid())
	{
		LogError(TEXT("No fact tag"));
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem::Get(World).ChangeFactValue(Fact, Value, ChangeType);
	}
	else
	{
		LogError(TEXT("No valid world"));
	}
}

#if WITH_EDITOR
FText UFVFlowNodeAddOn_SetFact::GetNodeTitle() const
{
	if (Fact.IsValid() && GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles)
	{
		FStringFormatOrderedArguments Args;
		Args.Add(Fact.ToString());
		Args.Add(UEnum::GetDisplayValueAsText(ChangeType).ToString());
		Args.Add(Value);

		return FText::FromString(FString::Format(TEXT("{0}: {1} {2}"), Args));
	}

	return Super::GetNodeTitle();
}

EDataValidationResult UFVFlowNodeAddOn_SetFact::ValidateNode()
{
	if (Fact.IsValid())
	{
		return EDataValidationResult::Valid;
	}

	ValidationLog.Error<UFlowNodeBase>(TEXT("Missing Tag"), this);
	return EDataValidationResult::Invalid;
}
#endif
