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

	if (AvailableActions.Num() > static_cast<int32>(EFVInteractionSlot::MAX))
	{
		Context.AddError(FText::Format(
			LOCTEXT("TooManyActions", "AvailableActions has {0} entries but only {1} input slots exist."),
			AvailableActions.Num(),
			static_cast<int32>(EFVInteractionSlot::MAX)));
		Result = EDataValidationResult::Invalid;
	}

	TMap<EFVInteractionSlot, int32> SlotUsage;

	for (int32 Index = 0; Index < AvailableActions.Num(); ++Index)
	{
		const UFVInteractionAction* Action = AvailableActions[Index];

		if (!Action)
		{
			Context.AddError(FText::Format(
				LOCTEXT("NullAction", "AvailableActions[{0}] is null."), Index));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (!Action->ActionTag.IsValid())
		{
			Context.AddError(FText::Format(
				LOCTEXT("InvalidActionTag", "AvailableActions[{0}] ('{1}') has an invalid ActionTag."),
				Index,
				FText::FromString(Action->GetName())));
			Result = EDataValidationResult::Invalid;
		}

		if (const int32* ExistingIndex = SlotUsage.Find(Action->Slot))
		{
			Context.AddError(FText::Format(
				LOCTEXT("DuplicateSlot", "AvailableActions[{0}] and [{1}] both bind to the same input slot."),
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
