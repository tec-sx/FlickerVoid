#pragma once

#include "Validation/InteractionCompileRule.h"

class FValidateInteractableResponseWiringRule : public IInteractionCompileRule
{
public:
	virtual FName GetRuleName() const override { return TEXT("ValidateInteractableResponseWiring"); }
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const override;
	virtual void Validate(const FInteractionCompileContext& Context) const override;
};
