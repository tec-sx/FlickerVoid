#include "Interactions/FVInteractionTargetConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionTargetConfig)

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "FVInteractionTargetConfig"

EDataValidationResult UFVInteractionTargetConfig::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (AvailableInteractions.Num() > static_cast<int32>(EFVInteractionSlot::MAX))
	{
		Context.AddError(FText::Format(
			LOCTEXT("TooManyActions", "AvailableInteractions has {0} entries but only {1} input slots exist."),
			AvailableInteractions.Num(),
			static_cast<int32>(EFVInteractionSlot::MAX)));
		Result = EDataValidationResult::Invalid;
	}

	TMap<EFVInteractionSlot, int32> SlotUsage;

	for (int32 Index = 0; Index < AvailableInteractions.Num(); ++Index)
	{
		const UFVInteractionConfig* Action = AvailableInteractions[Index];

		if (!Action)
		{
			Context.AddError(FText::Format(
				LOCTEXT("NullAction", "AvailableInteractions[{0}] is null."), Index));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (!Action->AbilityTag.IsValid())
		{
			Context.AddError(FText::Format(
				LOCTEXT("InvalidAbilityTag", "AvailableInteractions[{0}] ('{1}') has an invalid AbilityTag."),
				Index,
				FText::FromString(Action->GetName())));
			Result = EDataValidationResult::Invalid;
		}

		if (const int32* ExistingIndex = SlotUsage.Find(Action->Slot))
		{
			Context.AddError(FText::Format(
				LOCTEXT("DuplicateSlot", "AvailableInteractions[{0}] and [{1}] both bind to the same input slot."),
				*ExistingIndex,
				Index));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			SlotUsage.Add(Action->Slot, Index);
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

#endif
