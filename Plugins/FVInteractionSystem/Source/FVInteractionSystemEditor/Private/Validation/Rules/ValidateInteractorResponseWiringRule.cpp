#include "Validation/Rules/ValidateInteractorResponseWiringRule.h"

#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractorResponseComponent.h"
#include "Engine/Blueprint.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Validation/InteractionBlueprintComponentUtils.h"

bool FValidateInteractorResponseWiringRule::ShouldRun(const FInteractionCompileContext& Context) const
{
	return InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UFVInteractorResponseComponent::StaticClass());
}

void FValidateInteractorResponseWiringRule::Validate(const FInteractionCompileContext& Context) const
{
	if (InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UFVInteractorComponent::StaticClass()))
	{
		return;
	}

	TArray<const UActorComponent*> Templates;
	InteractionBlueprintComponentUtils::GetComponentTemplatesOfClass(Context.Blueprint, UFVInteractorResponseComponent::StaticClass(), Templates);

	for (const UActorComponent* Template : Templates)
	{
		Context.MessageLog.Error(*FString::Printf(
			TEXT("Interactor Response component '%s' can't find an FVInteractorComponent to bind to."),
			*Template->GetName()));
	}
}
