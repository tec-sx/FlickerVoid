#pragma once

#include "Validation/InteractionCompileRule.h"

class FRequireInteractorRule : public IInteractionCompileRule
{
public:
	virtual FName GetRuleName() const override { return TEXT("RequireInteractor"); }
	virtual void Validate(const FInteractionCompileContext& Context) const override;
};
