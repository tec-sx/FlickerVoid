#include "Scales/FVScaleConditions.h"

#include "Scales/FVScaleDefinition.h"
#include "Scales/FVScaleLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScaleConditions)

#define LOCTEXT_NAMESPACE "FVScaleConditions"

namespace FVScaleConditions
{
	FText ScaleName(const UFVScaleDefinition* Scale)
	{
		return Scale ? Scale->Display.Name : LOCTEXT("None", "<none>");
	}
}

bool UFVCondition_Scale::Evaluate_Implementation(const FFVConditionContext& Context) const
{
	if (!Scale)
	{
		return false;
	}

	const int32 Current = UFVScaleLibrary::GetValue(Context.WorldContext, Scale);
	return FVCompare::Compare(Current, Compare, Value);
}

bool UFVCondition_Scale::Score_Implementation(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	if (!Scale)
	{
		return false;
	}

	const int32 Current = UFVScaleLibrary::GetValue(Context.WorldContext, Scale);
	return FVCompare::Score(Current, Compare, Value, OutScore);
}

FText UFVCondition_Scale::GetDescription_Implementation() const
{
	return FText::Format(LOCTEXT("ScaleDesc", "{0} {1} {2}"), FVScaleConditions::ScaleName(Scale), FVCompare::GetDisplayName(Compare), FText::AsNumber(Value));
}

void UFVEffect_ModifyScale::Apply_Implementation(const FFVConditionContext& Context) const
{
	UFVScaleLibrary::ModifyScale(Context.WorldContext, Scale, Delta);
}

FText UFVEffect_ModifyScale::GetDescription_Implementation() const
{
	return FText::Format(LOCTEXT("ModifyScaleDesc", "{0} {1}"), FVScaleConditions::ScaleName(Scale), FText::AsNumber(Delta));
}

#undef LOCTEXT_NAMESPACE
