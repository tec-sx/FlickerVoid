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

		if (Interactable->GetOffers().IsEmpty())
		{
			Context.MessageLog.Warning(*FString::Printf(
				TEXT("Interactable component '%s' declares no offers and can never be interacted with."),
				*Interactable->GetName()));
		}

		TSet<FGameplayTag> SeenInputTags;

		for (const FInteractionOffer& Offer : Interactable->GetOffers())
		{
			if (!Offer.InputTag.IsValid())
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' has an offer with no input tag."),
					*Interactable->GetName()));
			}
			else if (SeenInputTags.Contains(Offer.InputTag))
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' declares more than one offer for input '%s'."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString()));
			}
			else
			{
				SeenInputTags.Add(Offer.InputTag);
			}

			if (Offer.RemainingUses == 0)
			{
				Context.MessageLog.Warning(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' is authored with zero remaining uses and can never execute."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString()));
			}

			if (Offer.InputMode == EInteractionInputMode::Hold && Offer.InteractionPeriod == 0.f)
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' uses Hold but has an interaction period of zero."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString()));
			}

			if (Offer.InputMode == EInteractionInputMode::Mash && Offer.RequiredPresses <= 1)
			{
				Context.MessageLog.Warning(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' uses Mash but requires %d presses."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString(),
					Offer.RequiredPresses));
			}

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

			if (Offer.RequiredTags.HasAny(Offer.BlockedTags))
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' has a tag present in both RequiredTags and BlockedTags, so it can never be satisfied."),
					*Interactable->GetName(),
					*Offer.InputTag.ToString()));
			}
		}
	}
}
