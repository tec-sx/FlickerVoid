#include "Interaction/FVInteractionEffects.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/FVInteractableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionEffects)

void FFVEffect_ActivateAbility::Apply(const FFVConditionContext& Context) const
{
	AActor* Instigator = Context.Instigator;
	if (!Instigator || !EventTag.IsValid())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Instigator;
	EventData.Target = Context.Target;
	EventData.OptionalObject = Context.Target ? Context.Target->FindComponentByClass<UFVInteractableComponent>() : nullptr;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Instigator, EventTag, EventData);
}

FText FFVEffect_ActivateAbility::GetDescription() const
{
	return FText::Format(NSLOCTEXT("FVInteractionEffects", "ActivateAbility", "Activate ability '{0}'"), FText::FromString(EventTag.ToString()));
}
