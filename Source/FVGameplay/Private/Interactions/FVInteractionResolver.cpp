#include "Interactions/FVInteractionResolver.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/FVAbilitySystemComponent.h"
#include "Interactions/FVInteractionAction.h"
#include "Interactions/FVInteractionTargetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionResolver)

#define LOCTEXT_NAMESPACE "FVInteractionResolver"

FFVResolvedInteractionSet UFVInteractionResolver::ResolveInteractions(
	UFVInteractionTargetComponent* Target,
	AActor* Instigator)
{
	FFVResolvedInteractionSet Resolved;

	if (!Target)
	{
		return Resolved;
	}

	const UFVAbilitySystemComponent* ASC = Cast<UFVAbilitySystemComponent>(
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Instigator));

	if (!ASC)
	{
		return Resolved;
	}

	for (UFVInteractionAction* Action : Target->GetAvailableActions())
	{
		if (!Action || Action->Slot >= EFVInteractionSlot::MAX)
		{
			continue;
		}

		const int32 SlotIndex = static_cast<int32>(Action->Slot);

		if (Resolved.Slots[SlotIndex].IsBound())
		{
			continue;
		}

		bool bAvailable = false;
		FGameplayTag FailureTag;

		if (!ASC->QueryAbilityAvailabilityByTag(Action->AbilityTag, bAvailable, FailureTag))
		{
			continue;
		}

		FFVResolvedInteraction Entry;
		Entry.Action = Action;
		Entry.Info = Action->CreateActionUIInfo();
		Entry.Info.bAvailable = bAvailable;

		if (!bAvailable)
		{
			Entry.Info.UnavailableReason = FailureTag.IsValid()
				? FText::FromName(FailureTag.GetTagName())
				: LOCTEXT("UnavailableGeneric", "Unavailable");
		}

		Resolved.Slots[SlotIndex] = MoveTemp(Entry);
	}

	return Resolved;
}

FFVResolvedInteraction UFVInteractionResolver::ResolveSlot(
	UFVInteractionTargetComponent* Target,
	AActor* Instigator,
	EFVInteractionSlot Slot)
{
	if (Slot >= EFVInteractionSlot::MAX)
	{
		return FFVResolvedInteraction();
	}

	return ResolveInteractions(Target, Instigator).GetSlot(Slot);
}

#undef LOCTEXT_NAMESPACE
