#pragma once

#include "CoreMinimal.h"
#include "Validation/InteractionCompileRule.h"

class FInteractionCompileRuleRegistry
{
public:
	static FInteractionCompileRuleRegistry& Get();

	void RegisterDefaultRules();
	void RegisterRule(TSharedRef<IInteractionCompileRule> Rule);
	void UnregisterRule(FName RuleName);
	void Reset();

	const TArray<TSharedRef<IInteractionCompileRule>>& GetRules() const { return Rules; }

private:
	TArray<TSharedRef<IInteractionCompileRule>> Rules;
};
