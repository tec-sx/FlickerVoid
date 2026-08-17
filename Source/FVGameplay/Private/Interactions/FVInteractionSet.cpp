#include "Interactions/FVInteractionSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionSet)

const FFVInteractionConfig& UFVInteractionSet::GetInteraction(EFVInteractionSlot Slot) const
{
	static const FFVInteractionConfig Empty;

	switch (Slot)
	{
	case EFVInteractionSlot::Primary:   return Primary;
	case EFVInteractionSlot::Secondary: return Secondary;
	case EFVInteractionSlot::Ternary:   return Ternary;
	default:                            return Empty;
	}
}
