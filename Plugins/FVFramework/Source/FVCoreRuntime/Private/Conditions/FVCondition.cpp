#include "Conditions/FVCondition.h"

#define LOCTEXT_NAMESPACE "FVCondition"

bool FFVConditionSet::Evaluate(const FFVConditionContext& Context) const
{
	const bool bAll = Mode == EFVConditionMode::All;

	for (const TInstancedStruct<FFVConditionBase>& Condition : Conditions)
	{
		const FFVConditionBase* Ptr = Condition.GetPtr();
		if (Ptr == nullptr)
		{
			continue;
		}

		const bool bPassed = Ptr->Evaluate(Context);
		if (bAll && !bPassed)
		{
			return false;
		}

		if (!bAll && bPassed)
		{
			return true;
		}
	}

	return bAll || Conditions.IsEmpty();
}

bool FFVConditionSet::GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	const bool bAll = Mode == EFVConditionMode::All;
	bool bFound = false;

	for (const TInstancedStruct<FFVConditionBase>& Condition : Conditions)
	{
		FFVConditionScore Score;
		const FFVConditionBase* Ptr = Condition.GetPtr();
		if (Ptr == nullptr || !Ptr->GetScore(Context, Score))
		{
			continue;
		}

		if (!bFound)
		{
			OutScore = Score;
		}
		else if (bAll)
		{
			OutScore.Threshold += Score.Threshold;
			OutScore.Excess += Score.Excess;
		}
		else if (OutScore < Score)
		{
			OutScore = Score;
		}
		bFound = true;
	}

	return bFound;
}

FText FFVConditionSet::GetDescription() const
{
	TArray<FText> Parts;
	for (const TInstancedStruct<FFVConditionBase>& Condition : Conditions)
	{
		if (const FFVConditionBase* Ptr = Condition.GetPtr())
		{
			const FText Description = Ptr->GetDescription();
			Parts.Add(Ptr->bInvert ? FText::Format(LOCTEXT("Not", "NOT {0}"), Description) : Description);
		}
	}

	const FText Separator = Mode == EFVConditionMode::All ? LOCTEXT("And", " AND ") : LOCTEXT("Or", " OR ");
	return FText::Join(Separator, Parts);
}

#undef LOCTEXT_NAMESPACE
