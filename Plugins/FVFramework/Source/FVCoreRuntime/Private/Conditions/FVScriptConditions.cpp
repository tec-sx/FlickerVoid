#include "Conditions/FVScriptConditions.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScriptConditions)

bool UFVScriptCondition::Evaluate_Implementation(const FFVConditionContext& Context) const
{
	return true;
}

FText UFVScriptCondition::GetDescription_Implementation() const
{
	return GetClass()->GetDisplayNameText();
}

UWorld* UFVScriptCondition::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

void UFVScriptEffect::Apply_Implementation(const FFVConditionContext& Context) const
{
}

FText UFVScriptEffect::GetDescription_Implementation() const
{
	return GetClass()->GetDisplayNameText();
}

UWorld* UFVScriptEffect::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

FText FFVCondition_Script::GetDescription() const
{
	return Condition != nullptr ? Condition->GetDescription() : FText::GetEmpty();
}

bool FFVCondition_Script::EvaluateImpl(const FFVConditionContext& Context) const
{
	return Condition == nullptr || Condition->Evaluate(Context);
}

void FFVEffect_Script::Apply(const FFVConditionContext& Context) const
{
	if (Effect != nullptr)
	{
		Effect->Apply(Context);
	}
}

FText FFVEffect_Script::GetDescription() const
{
	return Effect != nullptr ? Effect->GetDescription() : FText::GetEmpty();
}
