#include "Data/FVInteractableDefinition.h"

#include "Core/FVInteractionGameplayTags.h"
#include "FVInteractionSystemSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableDefinition)

#define LOCTEXT_NAMESPACE "FVInteractableDefinition"

UFVInteractableDefinition::UFVInteractableDefinition()
	: InteractableType(FVInteractionGameplayTags::Interactable)
{
	const FFVInteractableSettings& Defaults = UFVInteractionSystemSettings::Get().InteractableBaseSettings;
	CollisionChannel = Defaults.DefaultCollisionChannel;
	DetectionWeight = Defaults.DefaultInteractableWeight;
}

#if WITH_EDITOR
EDataValidationResult UFVInteractableDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Offers.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("NoOffers", "Interactable definition declares no offers and can never be interacted with."));
	}

	TSet<FGameplayTag> SeenInputTags;
	for (const FFVInteractionOffer& Offer : Offers)
	{
		const FText Input = FText::FromString(Offer.InputTag.ToString());

		if (!Offer.InputTag.IsValid())
		{
			Context.AddError(LOCTEXT("NoInput", "An offer has no input tag."));
			Result = EDataValidationResult::Invalid;
		}
		else if (SeenInputTags.Contains(Offer.InputTag))
		{
			Context.AddError(FText::Format(LOCTEXT("DupInput", "More than one offer for input '{0}'."), Input));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			SeenInputTags.Add(Offer.InputTag);
		}

		if (!Offer.ActionTag.IsValid())
		{
			Context.AddError(FText::Format(LOCTEXT("NoAction", "Offer '{0}' has no action tag."), Input));
			Result = EDataValidationResult::Invalid;
		}
		else if (!Offer.ActionTag.MatchesTag(FVInteractionGameplayTags::Interaction_Action))
		{
			Context.AddError(FText::Format(LOCTEXT("BadAction", "Offer '{0}' uses action tag '{1}', which is not under '{2}'."),
				Input, FText::FromString(Offer.ActionTag.ToString()),
				FText::FromString(FVInteractionGameplayTags::Interaction_Action.GetTag().ToString())));
			Result = EDataValidationResult::Invalid;
		}

		if (Offer.RemainingUses == 0)
		{
			Context.AddWarning(FText::Format(LOCTEXT("ZeroUses", "Offer '{0}' has zero remaining uses and can never execute."), Input));
		}

		for (const TInstancedStruct<FFVEffectBase>& Effect : Offer.Effects.Effects)
		{
			if (!Effect.IsValid())
			{
				Context.AddError(FText::Format(LOCTEXT("EmptyEffect", "Offer '{0}' has an empty effect entry."), Input));
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
