#include "Quest/Flow/FVFlowNode_SetQuestStage.h"

#include "FactDB/FVFactSubsystem.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"

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

FGameplayTag UFVFlowNode_SetQuestStage::MakeStageTag() const
{
	return QuestId.IsNone()
		? UFVQuestFactHelpers::MakeChapterStageTag(ChapterId)
		: UFVQuestFactHelpers::MakeQuestStageTag(ChapterId, QuestId);
}

void UFVFlowNode_SetQuestStage::ExecuteInput(const FName& PinName)
{
	const FGameplayTag StageTag = MakeStageTag();

	if (!StageTag.IsValid())
	{
		LogError(TEXT("Stage fact tag does not exist"));
	}
	else if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem::Get(World).ChangeFactValue(
			StageTag,
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
	if (ChapterId.IsNone())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Missing Chapter Id"), this);
		return EDataValidationResult::Invalid;
	}

	if (!MakeStageTag().IsValid())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Stage fact tag does not exist"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

FString UFVFlowNode_SetQuestStage::GetNodeDescription() const
{
	const FGameplayTag StageTag = MakeStageTag();
	if (!StageTag.IsValid())
	{
		return TEXT("None");
	}

	FStringFormatOrderedArguments Args;
	Args.Add(StageTag.ToString());
	Args.Add(UEnum::GetDisplayValueAsText(Stage).ToString());

	return FString::Format(TEXT("{0}\n= {1}"), Args);
}
#endif
