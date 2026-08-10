#include "Quest/Flow/FVFlowNode_AdvanceObjective.h"

#include "FactDB/FVFactSubsystem.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"
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

FGameplayTag UFVFlowNode_AdvanceObjective::MakeObjectiveTag() const
{
	return UFVQuestFactHelpers::MakeObjectiveTag(ChapterId, QuestId, ObjectiveId);
}

void UFVFlowNode_AdvanceObjective::ExecuteInput(const FName& PinName)
{
	const FGameplayTag ObjectiveTag = MakeObjectiveTag();

	if (!ObjectiveTag.IsValid())
	{
		LogError(TEXT("Objective fact tag does not exist"));
	}
	else if (const UWorld* World = GetWorld())
	{
		UFVFactSubsystem::Get(World).ChangeFactValue(
			ObjectiveTag,
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
	if (ChapterId.IsNone() || QuestId.IsNone() || ObjectiveId.IsNone())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Missing Chapter, Quest or Objective Id"), this);
		return EDataValidationResult::Invalid;
	}

	if (!MakeObjectiveTag().IsValid())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Objective fact tag does not exist"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

FString UFVFlowNode_AdvanceObjective::GetNodeDescription() const
{
	const FGameplayTag ObjectiveTag = MakeObjectiveTag();
	if (!ObjectiveTag.IsValid())
	{
		return TEXT("None");
	}

	FStringFormatOrderedArguments Args;
	Args.Add(ObjectiveTag.ToString());
	Args.Add(UEnum::GetDisplayValueAsText(NewState).ToString());

	return FString::Format(TEXT("{0}\n{1}"), Args);
}
#endif
