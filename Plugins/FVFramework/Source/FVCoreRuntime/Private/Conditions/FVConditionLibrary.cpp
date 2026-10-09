#include "Conditions/FVConditionLibrary.h"

FFVConditionContext UFVConditionLibrary::MakeContext(AActor* Instigator, AActor* Target)
{
	FFVConditionContext Context;
	Context.Instigator = Instigator;
	Context.Target = Target;
	Context.WorldContext = Instigator != nullptr ? static_cast<UObject*>(Instigator) : Target;
	return Context;
}

bool UFVConditionLibrary::EvaluateConditionSet(const FFVConditionSet& Set, AActor* Instigator, AActor* Target)
{
	return Set.Evaluate(MakeContext(Instigator, Target));
}

FText UFVConditionLibrary::DescribeConditionSet(const FFVConditionSet& Set)
{
	return Set.GetDescription();
}

void UFVConditionLibrary::ApplyEffects(const FFVEffectList& Effects, AActor* Instigator, AActor* Target)
{
	Effects.Apply(MakeContext(Instigator, Target));
}
