#include "Interaction/FVInteractionAction.h"

bool FFVInteractionAction::CheckRequirements(AActor* Instigator, FText& OutFirstUnmetReason) const
{
	for (const UFVInteractionRequirement* Requirement : Requirements)
	{
		if (!Requirement)
		{
			continue;
		}

		if (!Requirement->IsMet(Instigator))
		{
			OutFirstUnmetReason = Requirement->GetUnmetReason(Instigator);
			return false;
		}
	}

	return true;
}
