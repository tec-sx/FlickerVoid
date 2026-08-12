#include "Interactions/FVInteractionResolver.h"

#include "Interactions/FVInteractionAction.h"
#include "Interactions/FVInteractionTargetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionResolver)

FFVResolvedInteractionSet UFVInteractionResolver::ResolveInteractions(
	UFVInteractionTargetComponent* Target,
	const FGameplayTagContainer& InstigatorTags)
{
	FFVResolvedInteractionSet Resolved;

	if (!Target)
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

		// Config validation guarantees one action per slot; first binding wins if data is stale.
		if (Resolved.Slots[SlotIndex].IsBound())
		{
			continue;
		}

		FFVResolvedInteraction Entry;
		Entry.Action = Action;
		Entry.Info = Action->CreateActionUIInfo(InstigatorTags);

		Resolved.Slots[SlotIndex] = MoveTemp(Entry);
	}

	return Resolved;
}

FFVResolvedInteraction UFVInteractionResolver::ResolveSlot(
	UFVInteractionTargetComponent* Target,
	const FGameplayTagContainer& InstigatorTags,
	EFVInteractionSlot Slot)
{
	if (Slot >= EFVInteractionSlot::MAX)
	{
		return FFVResolvedInteraction();
	}

	return ResolveInteractions(Target, InstigatorTags).GetSlot(Slot);
}
