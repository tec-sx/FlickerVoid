#include "Validation/Rules/ValidateInteractableSetupRule.h"

#include "Components/FVInteractableComponent.h"
#include "Data/FVInteractableDefinition.h"
#include "Misc/DataValidation.h"

bool FValidateInteractableSetupRule::ShouldRun(const FInteractionCompileContext& Context) const
{
	return Context.Components.ContainsByPredicate([](const UActorComponent* Component)
	{
		return Component->IsA<UFVInteractableComponent>();
	});
}

void FValidateInteractableSetupRule::Validate(const FInteractionCompileContext& Context) const
{
	for (const UActorComponent* Component : Context.Components)
	{
		const UFVInteractableComponent* Interactable = Cast<UFVInteractableComponent>(Component);
		if (!Interactable)
		{
			continue;
		}

		const UFVInteractableDefinition* Definition = Interactable->Definition;
		if (!Definition)
		{
			Context.Error(FString::Printf(TEXT("Interactable component '%s' has no Definition assigned."), *Interactable->GetName()));
			continue;
		}

		FDataValidationContext DataContext;
		Definition->IsDataValid(DataContext);

		for (const FDataValidationContext::FIssue& Issue : DataContext.GetIssues())
		{
			const FString Message = FString::Printf(TEXT("Interactable component '%s' definition '%s': %s"),
				*Interactable->GetName(), *Definition->GetName(), *Issue.Message.ToString());

			if (Issue.Severity == EMessageSeverity::Error)
			{
				Context.Error(Message);
				continue;
			}
			Context.Warning(Message);
		}
	}
}
