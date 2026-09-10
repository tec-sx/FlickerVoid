#include "Validation/Rules/ValidateResponderWiringRule.h"

#include "Components/InteractableComponent.h"
#include "Components/InteractionResponderComponent.h"
#include "Components/InteractionResponseComponent.h"
#include "Engine/Blueprint.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Validation/InteractionBlueprintComponentUtils.h"

bool FValidateResponderWiringRule::ShouldRun(const FInteractionCompileContext& Context) const
{
	return InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UInteractionResponderComponent::StaticClass());
}

void FValidateResponderWiringRule::Validate(const FInteractionCompileContext& Context) const
{
	const bool bHasMailbox = InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UInteractionResponseComponent::StaticClass());
	const bool bHasInteractable = InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UInteractableComponent::StaticClass());

	if (bHasMailbox || bHasInteractable)
	{
		return;
	}

	TArray<const UActorComponent*> Templates;
	InteractionBlueprintComponentUtils::GetComponentTemplatesOfClass(Context.Blueprint, UInteractionResponderComponent::StaticClass(), Templates);

	for (const UActorComponent* Template : Templates)
	{
		Context.MessageLog.Error(*FString::Printf(
			TEXT("Responder component '%s' has neither an InteractionResponseComponent nor an InteractableComponent to bind to."),
			*Template->GetName()));
	}
}
