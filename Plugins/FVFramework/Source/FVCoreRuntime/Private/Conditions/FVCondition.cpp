#include "Conditions/FVCondition.h"

#include "FVCoreRuntime.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCondition)

#define LOCTEXT_NAMESPACE "FVCondition"

AActor* FFVConditionContext::GetActor(const bool bTarget, const UStruct* Requester) const
{
	AActor* Actor = bTarget ? Target.Get() : Instigator.Get();
#if !UE_BUILD_SHIPPING
	UE_CLOG(Actor == nullptr, LogFVCore, Warning, TEXT("%s needs a context %s but has none; it reads as empty."),
		Requester ? *Requester->GetName() : TEXT("Condition"), bTarget ? TEXT("target") : TEXT("instigator"));
#endif
	return Actor;
}

bool FFVConditionSet::Evaluate(const FFVConditionContext& Context) const
{
	const bool bAll = Mode == EFVConditionMode::All;

	for (const TInstancedStruct<FFVConditionBase>& Condition : Conditions)
	{
		const FFVConditionBase* Ptr = Condition.GetPtr();
		const bool bPassed = Ptr != nullptr && Ptr->Evaluate(Context);
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

FText FFVConditionSet::GetSetupError() const
{
	TArray<FText> Errors;
	for (int32 Index = 0; Index < Conditions.Num(); ++Index)
	{
		const FFVConditionBase* Ptr = Conditions[Index].GetPtr();
		if (Ptr == nullptr)
		{
			Errors.Add(FText::Format(LOCTEXT("NoType", "condition {0} has no type"), Index));
			continue;
		}

		const FText Error = Ptr->GetSetupError();
		if (!Error.IsEmpty())
		{
			const FText Type = FText::FromName(Conditions[Index].GetScriptStruct()->GetFName());
			Errors.Add(FText::Format(LOCTEXT("EntryError", "condition {0} ({1}): {2}"), Index, Type, Error));
		}
	}
	return FText::Join(LOCTEXT("ErrorSeparator", "; "), Errors);
}

bool UFVCondition::Check(const FFVConditionContext& Context) const
{
	return CanEvaluate(Context) && Evaluate(Context) != bInvert;
}

bool UFVCondition::GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	return !bInvert && CanEvaluate(Context) && Score(Context, OutScore);
}

bool UFVCondition::Evaluate_Implementation(const FFVConditionContext& Context) const
{
	UE_LOG(LogFVCore, Warning, TEXT("%s doesn't override Evaluate; it fails."), *GetClass()->GetName());
	return false;
}

bool UFVCondition::CanEvaluate_Implementation(const FFVConditionContext& Context) const
{
	const FText Error = GetSetupError();
	return Error.IsEmpty();
}

FText UFVCondition::GetSetupError_Implementation() const
{
	return FText::GetEmpty();
}

bool UFVCondition::Score_Implementation(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	return false;
}

FText UFVCondition::GetDescription_Implementation() const
{
	return GetClass()->GetDisplayNameText();
}

UWorld* UFVCondition::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

FText FFVCondition_Object::GetDescription() const
{
	if (Condition == nullptr)
	{
		return FText::GetEmpty();
	}

	const FText Description = Condition->GetDescription();
	return Condition->bInvert ? FText::Format(LOCTEXT("Not", "NOT {0}"), Description) : Description;
}

FText FFVCondition_Object::GetSetupError() const
{
	if (Condition == nullptr)
	{
		return LOCTEXT("NoObject", "no condition picked");
	}
	return Condition->GetSetupError();
}

bool FFVCondition_Object::CanEvaluate(const FFVConditionContext& Context) const
{
	return Condition != nullptr && Condition->CanEvaluate(Context);
}

bool FFVCondition_Object::EvaluateImpl(const FFVConditionContext& Context) const
{
	return Condition->Check(Context);
}

bool FFVCondition_Object::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	return Condition != nullptr && Condition->GetScore(Context, OutScore);
}

#undef LOCTEXT_NAMESPACE
