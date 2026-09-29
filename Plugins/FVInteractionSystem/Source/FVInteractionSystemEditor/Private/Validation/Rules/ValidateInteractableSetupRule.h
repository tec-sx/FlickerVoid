#pragma once

#include "Validation/InteractionCompileRule.h"

class FValidateInteractableSetupRule : public IInteractionCompileRule
{
public:
	virtual FName GetRuleName() const override { return TEXT("ValidateInteractableSetup"); }
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const override;
	virtual void Validate(const FInteractionCompileContext& Context) const override;
};
