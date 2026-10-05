#include "Conditions/FVAttributeConditions.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeConditions)

#define LOCTEXT_NAMESPACE "FVAttributeConditions"

namespace
{
	UFVAttributeComponent* FindComponent(const FFVConditionContext& Context, bool bTarget)
	{
		return UFVAttributeComponent::Find(bTarget ? Context.Target : Context.Instigator);
	}

	FText AttributeName(const UFVAttributeDefinition* Attribute)
	{
		return Attribute != nullptr ? Attribute->Display.Name : FText::GetEmpty();
	}
}

FText FFVCondition_Attribute::GetDescription() const
{
	FText Comparison;
	switch (Compare)
	{
	case EFVCompare::AtLeast: Comparison = LOCTEXT("AtLeast", "at least"); break;
	case EFVCompare::AtMost: Comparison = LOCTEXT("AtMost", "at most"); break;
	default: Comparison = LOCTEXT("Exactly", "exactly"); break;
	}

	return FText::Format(LOCTEXT("AttributeDesc", "{0} {1} {2}"), AttributeName(Attribute), Comparison, FText::AsNumber(Value));
}

bool FFVCondition_Attribute::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVAttributeComponent* Component = FindComponent(Context, bCheckTarget);
	if (Component == nullptr || Attribute == nullptr)
	{
		return false;
	}

	const float Current = Component->GetValue(Attribute);

	switch (Compare)
	{
	case EFVCompare::AtLeast: return Current >= Value;
	case EFVCompare::AtMost: return Current <= Value;
	default: return FMath::IsNearlyEqual(Current, Value);
	}
}

void FFVEffect_ModifyAttribute::Apply(const FFVConditionContext& Context) const
{
	if (UFVAttributeComponent* Component = FindComponent(Context, bApplyToTarget))
	{
		Component->ModifyBaseValue(Attribute, Delta);
	}
}

FText FFVEffect_ModifyAttribute::GetDescription() const
{
	return FText::Format(LOCTEXT("ModifyDesc", "{0} {1}"), AttributeName(Attribute), FText::AsNumber(Delta));
}

void FFVEffect_SetAttribute::Apply(const FFVConditionContext& Context) const
{
	if (UFVAttributeComponent* Component = FindComponent(Context, bApplyToTarget))
	{
		Component->SetBaseValue(Attribute, Value);
	}
}

FText FFVEffect_SetAttribute::GetDescription() const
{
	return FText::Format(LOCTEXT("SetDesc", "{0} = {1}"), AttributeName(Attribute), FText::AsNumber(Value));
}

#undef LOCTEXT_NAMESPACE
