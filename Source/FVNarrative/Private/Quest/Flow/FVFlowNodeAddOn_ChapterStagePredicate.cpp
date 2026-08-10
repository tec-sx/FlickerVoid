#include "Quest/Flow/FVFlowNodeAddOn_ChapterStagePredicate.h"

#include "FlowSettings.h"
#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNodeAddOn_ChapterStagePredicate)

UFVFlowNodeAddOn_ChapterStagePredicate::UFVFlowNodeAddOn_ChapterStagePredicate()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::AddOn_Predicate;
	Category = TEXT("Chapter");
#endif
}

FFVFactCondition UFVFlowNodeAddOn_ChapterStagePredicate::MakeCondition() const
{
	FFVFactCondition Condition;
	Condition.Tag = UFVQuestFactHelpers::MakeChapterStageTag(ChapterId);
	Condition.Operator = Operator;
	Condition.WantedValue = UFVQuestFactHelpers::QuestStageToValue(Stage);

	return Condition;
}

bool UFVFlowNodeAddOn_ChapterStagePredicate::EvaluatePredicate_Implementation() const
{
	if (const UWorld* World = GetWorld())
	{
		return UFVFactSubsystem::Get(World).CheckFactCondition(MakeCondition());
	}

	LogError(TEXT("No valid world"));
	return false;
}

#if WITH_EDITOR
FText UFVFlowNodeAddOn_ChapterStagePredicate::GetNodeTitle() const
{
	if (!ChapterId.IsNone() && GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles)
	{
		const FFVFactCondition Condition = MakeCondition();
		if (Condition.IsValid())
		{
			return FText::FromString(Condition.ToString());
		}
	}

	return Super::GetNodeTitle();
}

EDataValidationResult UFVFlowNodeAddOn_ChapterStagePredicate::ValidateNode()
{
	if (ChapterId.IsNone())
	{
		ValidationLog.Error<UFlowNodeBase>(TEXT("Missing Chapter Id"), this);
		return EDataValidationResult::Invalid;
	}

	if (!MakeCondition().IsValid())
	{
		ValidationLog.Error<UFlowNodeBase>(TEXT("Chapter stage fact tag does not exist"), this);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
