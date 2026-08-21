#pragma once

#include "Validation/InteractionCompileRule.h"

/**
 * Verifies that every authored UInteractableComponent resolves a focus profile
 * from the interaction settings and that each of its offers carries a valid action tag.
 */
class FValidateInteractableSetupRule : public IInteractionCompileRule
{
public:
	virtual FName GetRuleName() const override { return TEXT("ValidateInteractableSetup"); }
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const override;
	virtual void Validate(const FInteractionCompileContext& Context) const override;
};
