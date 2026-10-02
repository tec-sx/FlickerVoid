#include "Validation/Rules/ValidateInteractableSetupRule.h"

#include "Components/FVInteractableComponent.h"
#include "Data/FVInteractableDefinition.h"
#include "Engine/Blueprint.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Misc/DataValidation.h"
#include "Validation/InteractionBlueprintComponentUtils.h"

bool FValidateInteractableSetupRule::ShouldRun(const FInteractionCompileContext& Context) const
{
	return InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UFVInteractableComponent::StaticClass());
}

void FValidateInteractableSetupRule::Validate(const FInteractionCompileContext& Context) const
{
	TArray<const UActorComponent*> Templates;
	InteractionBlueprintComponentUtils::GetComponentTemplatesOfClass(Context.Blueprint, UFVInteractableComponent::StaticClass(), Templates);

	for (const UActorComponent* Template : Templates)
	{
		const UFVInteractableComponent* Interactable = Cast<UFVInteractableComponent>(Template);
		if (!Interactable)
		{
			continue;
		}

		const UFVInteractableDefinition* Definition = Interactable->Definition;
		if (!Definition)
		{
			Context.MessageLog.Error(*FString::Printf(
				TEXT("Interactable component '%s' has no Definition assigned."),
				*Interactable->GetName()));
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
				Context.MessageLog.Error(*Message);
			}
			else
			{
				Context.MessageLog.Warning(*Message);
			}
		}
	}
}
