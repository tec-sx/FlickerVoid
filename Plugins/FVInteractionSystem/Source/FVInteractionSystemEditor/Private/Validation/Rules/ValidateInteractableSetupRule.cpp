#include "Validation/Rules/ValidateInteractableSetupRule.h"

#include "Components/InteractableComponent.h"
#include "Core/InteractionGameplayTags.h"
#include "FVInteractionSystemSettings.h"
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

	const UFVInteractionSystemSettings* Settings = GetDefault<UFVInteractionSystemSettings>();

	for (const UActorComponent* Template : Templates)
	{
		const UInteractableComponent* Interactable = Cast<UInteractableComponent>(Template);
		if (!Interactable)
		{
			continue;
		}

		const FName ProfileName = Interactable->GetFocusProfileName();
		if (ProfileName.IsNone() || !Settings->FocusProfiles.Contains(ProfileName))
		{
			Context.MessageLog.Error(*FString::Printf(
				TEXT("Interactable component '%s' references focus profile '%s', which is not defined in the FV Interaction System settings."),
				*Interactable->GetName(),
				*ProfileName.ToString()));
		}

		for (const TPair<FGameplayTag, FInteractionOffer>& Pair : Interactable->GetOffers())
		{
			if (!Pair.Value.ActionTag.IsValid())
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' has an offer for input '%s' with no action tag."),
					*Interactable->GetName(),
					*Pair.Key.ToString()));
			}
			else if (!Pair.Value.ActionTag.MatchesTag(InteractionTags::Interaction_Action))
			{
				Context.MessageLog.Error(*FString::Printf(
					TEXT("Interactable component '%s' offer '%s' uses action tag '%s', which is not under '%s'."),
					*Interactable->GetName(),
					*Pair.Key.ToString(),
					*Pair.Value.ActionTag.ToString(),
					*InteractionTags::Interaction_Action.GetTag().ToString()));
			}
		}
	}
}
