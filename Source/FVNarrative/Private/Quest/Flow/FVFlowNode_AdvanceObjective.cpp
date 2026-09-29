#include "Quest/Flow/FVFlowNode_AdvanceObjective.h"

#include "FactDB/FVFactSubsystem.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"
#include "FactDB/Flow/FVFlowNodeAddOn_SetFact.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Quest/FVQuestFactHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_AdvanceObjective)

UFVFlowNode_AdvanceObjective::UFVFlowNode_AdvanceObjective()
{
	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(TEXT("Out"))};

#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Fact;
	Category = TEXT("Quest");
#endif
}

EFlowAddOnAcceptResult UFVFlowNode_AdvanceObjective::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate,
	const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate) || (IsValid(AddOnTemplate) && AddOnTemplate->IsA<UFVFlowNodeAddOn_SetFact>()))
	{
		return EFlowAddOnAcceptResult::TentativeAccept;
	}

	return Super::AcceptFlowNodeAddOnChild_Implementation(AddOnTemplate, AdditionalAddOnsToAssumeAreChildren);
}

void UFVFlowNode_AdvanceObjective::ExecuteInput(const FName& PinName)
{
	if (!UFVQuestFactHelpers::IsObjectiveTag(Objective))
	{
		LogError(TEXT("Objective must be a Fact.Quest.<Chapter>.<Quest>.Objective.<Id> tag"));
	}
	else if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem::Get(World).ChangeFactValue(
			Objective,
			static_cast<int32>(NewState),
			EFVFactValueChangeType::Set);
	}
	else
	{
		LogError(TEXT("No valid world"));
	}

	TriggerFirstOutput(true);
}

#if WITH_EDITOR
EDataValidationResult UFVFlowNode_AdvanceObjective::ValidateNode()
{
	if (!Objective.IsValid())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Missing Objective tag"), this);
		return EDataValidationResult::Invalid;
	}

	if (!UFVQuestFactHelpers::IsObjectiveTag(Objective))
	{
		ValidationLog.Error<UFlowNode>(TEXT("Objective must be Fact.Quest.<Chapter>.<Quest>.Objective.<Id>"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

FString UFVFlowNode_AdvanceObjective::GetNodeDescription() const
{
	if (!Objective.IsValid())
	{
		return TEXT("None");
	}

	if (!UFVQuestFactHelpers::IsObjectiveTag(Objective))
	{
		return FString::Printf(TEXT("%s\nNot an Objective tag"), *Objective.ToString());
	}

	FStringFormatOrderedArguments Args;
	Args.Add(Objective.ToString());
	Args.Add(UEnum::GetDisplayValueAsText(NewState).ToString());

	return FString::Format(TEXT("{0}\n{1}"), Args);
}
#endif
