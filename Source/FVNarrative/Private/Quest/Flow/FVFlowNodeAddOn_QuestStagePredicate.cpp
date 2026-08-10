#include "Quest/Flow/FVFlowNodeAddOn_QuestStagePredicate.h"

#include "FlowSettings.h"
#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNodeAddOn_QuestStagePredicate)

UFVFlowNodeAddOn_QuestStagePredicate::UFVFlowNodeAddOn_QuestStagePredicate()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::AddOn_Predicate;
	Category = TEXT("Quest");
#endif
}

FFVFactCondition UFVFlowNodeAddOn_QuestStagePredicate::MakeCondition() const
{
	FFVFactCondition Condition;
	Condition.Tag = UFVQuestFactHelpers::MakeQuestStageTag(ChapterId, QuestId);
	Condition.Operator = Operator;
	Condition.WantedValue = UFVQuestFactHelpers::QuestStageToValue(Stage);

	return Condition;
}

bool UFVFlowNodeAddOn_QuestStagePredicate::EvaluatePredicate_Implementation() const
{
	if (const UWorld* World = GetWorld())
	{
		return UFVFactSubsystem::Get(World).CheckFactCondition(MakeCondition());
	}

	LogError(TEXT("No valid world"));
	return false;
}

#if WITH_EDITOR
FText UFVFlowNodeAddOn_QuestStagePredicate::GetNodeTitle() const
{
	if (!ChapterId.IsNone() && !QuestId.IsNone() && GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles)
	{
		const FFVFactCondition Condition = MakeCondition();
		if (Condition.IsValid())
		{
			return FText::FromString(Condition.ToString());
		}
	}

	return Super::GetNodeTitle();
}

EDataValidationResult UFVFlowNodeAddOn_QuestStagePredicate::ValidateNode()
{
	if (ChapterId.IsNone() || QuestId.IsNone())
	{
		ValidationLog.Error<UFlowNodeBase>(TEXT("Missing Chapter Id or Quest Id"), this);
		return EDataValidationResult::Invalid;
	}

	if (!MakeCondition().IsValid())
	{
		ValidationLog.Error<UFlowNodeBase>(TEXT("Quest stage fact tag does not exist"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
