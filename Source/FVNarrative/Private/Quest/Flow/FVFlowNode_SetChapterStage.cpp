#include "Quest/Flow/FVFlowNode_SetChapterStage.h"

#include "FactDB/FVFactSubsystem.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"
#include "FactDB/Flow/FVFlowNodeAddOn_SetFact.h"
#include "Interfaces/FlowPredicateInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_SetChapterStage)

UFVFlowNode_SetChapterStage::UFVFlowNode_SetChapterStage()
{
	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(TEXT("Out"))};

#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Fact;
	Category = TEXT("Quest");
#endif
}

EFlowAddOnAcceptResult UFVFlowNode_SetChapterStage::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate,
	const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate) || (IsValid(AddOnTemplate) && AddOnTemplate->IsA<UFVFlowNodeAddOn_SetFact>()))
	{
		return EFlowAddOnAcceptResult::TentativeAccept;
	}

	return Super::AcceptFlowNodeAddOnChild_Implementation(AddOnTemplate, AdditionalAddOnsToAssumeAreChildren);
}

void UFVFlowNode_SetChapterStage::ExecuteInput(const FName& PinName)
{
	if (!UFVQuestFactHelpers::IsChapterStageTag(ChapterStage))
	{
		LogError(TEXT("Chapter Stage must be a Fact.Chapter.<Chapter>.Stage tag"));
	}
	else if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem::Get(World).ChangeFactValue(
			ChapterStage,
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
EDataValidationResult UFVFlowNode_SetChapterStage::ValidateNode()
{
	if (!ChapterStage.IsValid())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Missing Chapter Stage tag"), this);
		return EDataValidationResult::Invalid;
	}

	if (!UFVQuestFactHelpers::IsChapterStageTag(ChapterStage))
	{
		ValidationLog.Error<UFlowNode>(TEXT("Chapter Stage must be Fact.Chapter.<Chapter>.Stage"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

FString UFVFlowNode_SetChapterStage::GetNodeDescription() const
{
	if (!ChapterStage.IsValid())
	{
		return TEXT("None");
	}

	if (!UFVQuestFactHelpers::IsChapterStageTag(ChapterStage))
	{
		return FString::Printf(TEXT("%s\nNot a chapter Stage tag"), *ChapterStage.ToString());
	}

	FStringFormatOrderedArguments Args;
	Args.Add(ChapterStage.ToString());
	Args.Add(UEnum::GetDisplayValueAsText(Stage).ToString());

	return FString::Format(TEXT("{0}\n= {1}"), Args);
}
#endif
