#include "Conditions/FVAttributeConditions.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "Attributes/FVAttributeLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeConditions)

#define LOCTEXT_NAMESPACE "FVAttributeConditions"

namespace
{
	FText AttributeName(const UFVAttributeDefinition* Attribute)
	{
		return Attribute != nullptr ? Attribute->Display.Name : FText::GetEmpty();
	}

	FText EffectName(const TSubclassOf<UGameplayEffect>& Effect)
	{
		return Effect != nullptr ? Effect->GetDisplayNameText() : FText::GetEmpty();
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

bool FFVCondition_Attribute::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	const AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	if (Compare != EFVCompare::AtLeast || !UFVAttributeLibrary::HasAttribute(Actor, Attribute))
	{
		return false;
	}

	OutScore = FFVConditionScore::AtLeast(UFVAttributeLibrary::GetValue(Actor, Attribute), Value);
	return true;
}

bool FFVCondition_Attribute::EvaluateImpl(const FFVConditionContext& Context) const
{
	const AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	if (!UFVAttributeLibrary::HasAttribute(Actor, Attribute))
	{
		return false;
	}

	const float Current = UFVAttributeLibrary::GetValue(Actor, Attribute);

	switch (Compare)
	{
	case EFVCompare::AtLeast: return Current >= Value;
	case EFVCompare::AtMost: return Current <= Value;
	default: return FMath::IsNearlyEqual(Current, Value);
	}
}

void FFVEffect_ModifyAttribute::Apply(const FFVConditionContext& Context) const
{
	UFVAttributeLibrary::AddToBaseValue(Context.GetActor(bApplyToTarget, StaticStruct()), Attribute, Delta);
}

FText FFVEffect_ModifyAttribute::GetDescription() const
{
	return FText::Format(LOCTEXT("ModifyDesc", "{0} {1}"), AttributeName(Attribute), FText::AsNumber(Delta));
}

void FFVEffect_SetAttribute::Apply(const FFVConditionContext& Context) const
{
	UFVAttributeLibrary::SetBaseValue(Context.GetActor(bApplyToTarget, StaticStruct()), Attribute, Value);
}

FText FFVEffect_SetAttribute::GetDescription() const
{
	return FText::Format(LOCTEXT("SetDesc", "{0} = {1}"), AttributeName(Attribute), FText::AsNumber(Value));
}

void FFVEffect_ApplyGameplayEffect::Apply(const FFVConditionContext& Context) const
{
	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context.GetActor(bApplyToTarget, StaticStruct()));
	if (AbilitySystem == nullptr || Effect == nullptr)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
	EffectContext.AddInstigator(Context.Instigator, Context.Instigator);
	AbilitySystem->BP_ApplyGameplayEffectToSelf(Effect, Level, EffectContext);
}

FText FFVEffect_ApplyGameplayEffect::GetDescription() const
{
	return FText::Format(LOCTEXT("ApplyEffectDesc", "Apply {0}"), EffectName(Effect));
}

void FFVEffect_RemoveGameplayEffect::Apply(const FFVConditionContext& Context) const
{
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Context.GetActor(bApplyToTarget, StaticStruct())))
	{
		if (Effect != nullptr)
		{
			AbilitySystem->RemoveActiveGameplayEffectBySourceEffect(Effect, nullptr);
		}
	}
}

FText FFVEffect_RemoveGameplayEffect::GetDescription() const
{
	return FText::Format(LOCTEXT("RemoveEffectDesc", "Remove {0}"), EffectName(Effect));
}

#undef LOCTEXT_NAMESPACE
