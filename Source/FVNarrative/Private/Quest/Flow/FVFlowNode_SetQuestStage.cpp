#include "Quest/Flow/FVFlowNode_SetQuestStage.h"

#include "FactDB/FVFactSubsystem.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"
#include "FactDB/Flow/FVFlowNodeAddOn_SetFact.h"
#include "Interfaces/FlowPredicateInterface.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_SetQuestStage)

UFVFlowNode_SetQuestStage::UFVFlowNode_SetQuestStage()
{
	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(TEXT("Out"))};

#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Fact;
	Category = TEXT("Quest");
#endif
}

EFlowAddOnAcceptResult UFVFlowNode_SetQuestStage::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate,
	const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate) || (IsValid(AddOnTemplate) && AddOnTemplate->IsA<UFVFlowNodeAddOn_SetFact>()))
	{
		return EFlowAddOnAcceptResult::TentativeAccept;
	}

	return Super::AcceptFlowNodeAddOnChild_Implementation(AddOnTemplate, AdditionalAddOnsToAssumeAreChildren);
}

void UFVFlowNode_SetQuestStage::ExecuteInput(const FName& PinName)
{
	if (!UFVQuestFactHelpers::IsQuestStageTag(QuestStage))
	{
		LogError(TEXT("Quest Stage must be a Fact.Quest.<Chapter>.<Quest>.Stage tag"));
	}
	else if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem::Get(World).ChangeFactValue(
			QuestStage,
			UFVQuestFactHelpers::QuestStageToValue(Stage),
			EFVFactValueChangeType::Set);
	}
	else
	{
		LogError(TEXT("No valid world"));
	}

	TriggerFirstOutput(true);
}

#if WITH_EDITOR
EDataValidationResult UFVFlowNode_SetQuestStage::ValidateNode()
{
	if (!QuestStage.IsValid())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Missing Quest Stage tag"), this);
		return EDataValidationResult::Invalid;
	}

	if (!UFVQuestFactHelpers::IsQuestStageTag(QuestStage))
	{
		ValidationLog.Error<UFlowNode>(TEXT("Quest Stage must be Fact.Quest.<Chapter>.<Quest>.Stage"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

FString UFVFlowNode_SetQuestStage::GetNodeDescription() const
{
	if (!QuestStage.IsValid())
	{
		return TEXT("None");
	}

	if (!UFVQuestFactHelpers::IsQuestStageTag(QuestStage))
	{
		return FString::Printf(TEXT("%s\nNot a quest Stage tag"), *QuestStage.ToString());
	}

	FStringFormatOrderedArguments Args;
	Args.Add(QuestStage.ToString());
	Args.Add(UEnum::GetDisplayValueAsText(Stage).ToString());

	return FString::Format(TEXT("{0}\n= {1}"), Args);
}
#endif
