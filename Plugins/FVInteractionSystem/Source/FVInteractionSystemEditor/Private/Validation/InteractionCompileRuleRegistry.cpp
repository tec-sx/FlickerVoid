#include "Validation/InteractionCompileRuleRegistry.h"

#include "Validation/Rules/ValidateInteractableSetupRule.h"
#include "Validation/Rules/ValidateResponderWiringRule.h"

FInteractionCompileRuleRegistry& FInteractionCompileRuleRegistry::Get()
{
	static FInteractionCompileRuleRegistry Instance;
	return Instance;
}

void FInteractionCompileRuleRegistry::RegisterDefaultRules()
{
	RegisterRule(MakeShared<FValidateInteractableSetupRule>());
	RegisterRule(MakeShared<FValidateResponderWiringRule>());
}

void FInteractionCompileRuleRegistry::RegisterRule(TSharedRef<IInteractionCompileRule> Rule)
{
	const FName RuleName = Rule->GetRuleName();
	UnregisterRule(RuleName);
	Rules.Add(MoveTemp(Rule));
}

void FInteractionCompileRuleRegistry::UnregisterRule(FName RuleName)
{
	Rules.RemoveAll([RuleName](const TSharedRef<IInteractionCompileRule>& Rule)
	{
		return Rule->GetRuleName() == RuleName;
	});
}

void FInteractionCompileRuleRegistry::Reset()
{
	Rules.Reset();
}
