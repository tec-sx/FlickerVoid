#include "Conditions/FVConditionStatics.h"

FFVConditionContext UFVConditionStatics::MakeContext(AActor* Instigator, AActor* Target)
{
	FFVConditionContext Context;
	Context.Instigator = Instigator;
	Context.Target = Target;
	Context.WorldContext = Instigator != nullptr ? static_cast<UObject*>(Instigator) : Target;
	return Context;
}

bool UFVConditionStatics::EvaluateConditionSet(const FFVConditionSet& Set, AActor* Instigator, AActor* Target)
{
	return Set.Evaluate(MakeContext(Instigator, Target));
}

FText UFVConditionStatics::DescribeConditionSet(const FFVConditionSet& Set)
{
	return Set.GetDescription();
}

void UFVConditionStatics::ApplyEffects(const FFVEffectList& Effects, AActor* Instigator, AActor* Target)
{
	Effects.Apply(MakeContext(Instigator, Target));
}
