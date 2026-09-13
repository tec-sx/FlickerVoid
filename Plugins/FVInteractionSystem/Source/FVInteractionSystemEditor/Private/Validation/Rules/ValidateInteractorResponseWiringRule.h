#pragma once

#include "Validation/InteractionCompileRule.h"

class FValidateInteractorResponseWiringRule : public IInteractionCompileRule
{
public:
	virtual FName GetRuleName() const override { return TEXT("ValidateInteractorResponseWiring"); }
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const override;
	virtual void Validate(const FInteractionCompileContext& Context) const override;
};
