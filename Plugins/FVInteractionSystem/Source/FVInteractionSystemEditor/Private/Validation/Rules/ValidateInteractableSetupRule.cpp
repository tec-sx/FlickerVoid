#include "Validation/Rules/ValidateInteractableSetupRule.h"

#include "Components/InteractableComponent.h"
#include "Core/InteractionGameplayTags.h"
#include "Engine/Blueprint.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Validation/InteractionBlueprintComponentUtils.h"

bool FValidateInteractableSetupRule::ShouldRun(const FInteractionCompileContext& Context) const
{
	return InteractionBlueprintComponentUtils::HasComponentOfClass(Context.Blueprint, UInteractableComponent::StaticClass());
}

void FValidateInteractableSetupRule::Validate(const FInteractionCompileContext& Context) const
{
	TArray<const UActorComponent*> Templates;
	InteractionBlueprintComponentUtils::GetComponentTemplatesOfClass(Context.Blueprint, UInteractableComponent::StaticClass(), Templates);

	for (const UActorComponent* Template : Templates)
	{
		const UInteractableComponent* Interactable = Cast<UInteractableComponent>(Template);
		if (!Interactable)
		{
			continue;
		}

		for (const FInteractionOffer& Offer : Interactable->GetOffers())
		{
			if (!Offer.ActionTag.IsValid())
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' has an offer for input '%s' with no action tag."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString()));
			}
			else if (!Offer.ActionTag.MatchesTag(InteractionTags::Interaction_Action))
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' uses action tag '%s', which is not under '%s'."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString(),
					*Offer.ActionTag.ToString(),
					*InteractionTags::Interaction_Action.GetTag().ToString()));
			}

			if (Offer.Requirements.Contains(nullptr))
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' has an empty requirement entry."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString()));
			}
		}
	}
}
