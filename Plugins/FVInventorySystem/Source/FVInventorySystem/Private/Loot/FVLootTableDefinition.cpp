#include "Loot/FVLootTableDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLootTableDefinition)

#define LOCTEXT_NAMESPACE "FVLootTableDefinition"

#if WITH_EDITOR
EDataValidationResult UFVLootTableDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Rule of three: a longer list is harder to balance than a nested table.
	if (Entries.Num() > 3)
	{
		Context.AddWarning(LOCTEXT("TooManyEntries", "Keep at most three entries; nest tables for more."));
	}

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const FFVLootEntry& Entry = Entries[Index];

		if (!Entry.Item && !Entry.Table)
		{
			Context.AddError(FText::Format(LOCTEXT("EmptyEntry", "Entry {0} has neither an Item nor a Table."), Index));
			Result = EDataValidationResult::Invalid;
		}
		else if (Entry.Item && Entry.Table)
		{
			Context.AddWarning(FText::Format(LOCTEXT("ItemAndTable", "Entry {0} has both an Item and a Table; only the Table is rolled."), Index));
		}

		if (Entry.Table.Get() == this)
		{
			Context.AddError(FText::Format(LOCTEXT("SelfTable", "Entry {0} rolls this table itself."), Index));
			Result = EDataValidationResult::Invalid;
		}

		if (Entry.MinQuantity > Entry.MaxQuantity)
		{
			Context.AddError(FText::Format(LOCTEXT("QuantityRange", "Entry {0} has Min Quantity above Max Quantity."), Index));
			Result = EDataValidationResult::Invalid;
		}
	}

	if (MinMoney > MaxMoney)
	{
		Context.AddError(LOCTEXT("MoneyRange", "Min Money is above Max Money."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
