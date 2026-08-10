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
	Condition.Tag = QuestStage;
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
	if (QuestStage.IsValid() && GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles)
	{
		if (!UFVQuestFactHelpers::IsQuestStageTag(QuestStage))
		{
			return FText::FromString(FString::Printf(TEXT("%s (not a quest Stage tag)"), *QuestStage.ToString()));
		}

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
	if (!QuestStage.IsValid())
	{
		ValidationLog.Error<UFlowNodeBase>(TEXT("Missing Quest Stage tag"), this);
		return EDataValidationResult::Invalid;
	}

	if (!UFVQuestFactHelpers::IsQuestStageTag(QuestStage))
	{
		ValidationLog.Error<UFlowNodeBase>(TEXT("Quest Stage must be Fact.Quest.<Chapter>.<Quest>.Stage"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
