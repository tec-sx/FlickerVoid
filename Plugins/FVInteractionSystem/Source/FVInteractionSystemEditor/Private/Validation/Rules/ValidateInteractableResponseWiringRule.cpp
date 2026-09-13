#include "Validation/Rules/ValidateInteractableResponseWiringRule.h"

#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractableResponseComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractorResponseComponent.h"
#include "Engine/Blueprint.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Validation/InteractionBlueprintComponentUtils.h"

bool FValidateInteractableResponseWiringRule::ShouldRun(const FInteractionCompileContext& Context) const
{
	return InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UFVInteractableResponseComponent::StaticClass());
}

void FValidateInteractableResponseWiringRule::Validate(const FInteractionCompileContext& Context) const
{
	if (InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UFVInteractableComponent::StaticClass()))
	{
		return;
	}

	TArray<const UActorComponent*> Templates;
	InteractionBlueprintComponentUtils::GetComponentTemplatesOfClass(Context.Blueprint, UFVInteractableResponseComponent::StaticClass(), Templates);

	for (const UActorComponent* Template : Templates)
	{
		Context.MessageLog.Error(*FString::Printf(
			TEXT("Interactable Response component '%s' can't find an FVInteractableComponent to bind to."),
			*Template->GetName()));
	}
}
