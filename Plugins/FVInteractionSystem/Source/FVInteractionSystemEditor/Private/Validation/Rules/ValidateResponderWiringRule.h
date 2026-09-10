#pragma once

#include "Validation/InteractionCompileRule.h"

class FValidateResponderWiringRule : public IInteractionCompileRule
{
public:
	virtual FName GetRuleName() const override { return TEXT("ValidateResponderWiring"); }
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const override;
	virtual void Validate(const FInteractionCompileContext& Context) const override;
};
