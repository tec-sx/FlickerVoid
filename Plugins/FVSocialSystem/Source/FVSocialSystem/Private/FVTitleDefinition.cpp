#include "FVTitleDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVTitleDefinition)

#define LOCTEXT_NAMESPACE "FVTitleDefinition"

#if WITH_EDITOR
EDataValidationResult UFVTitleDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// A title that ends up replacing itself through Replaces would block itself forever.
	TArray<const UFVTitleDefinition*> Open = { this };
	TSet<const UFVTitleDefinition*> Visited;
	while (!Open.IsEmpty())
	{
		const UFVTitleDefinition* Current = Open.Pop();
		for (const TObjectPtr<UFVTitleDefinition>& Replaced : Current->Replaces)
		{
			if (Replaced.Get() == this)
			{
				Context.AddError(FText::Format(LOCTEXT("ReplacesCycle", "{0} replaces itself through its Replaces chain."), FText::FromName(GetFName())));
				return EDataValidationResult::Invalid;
			}
			if (Replaced != nullptr && !Visited.Contains(Replaced.Get()))
			{
				Visited.Add(Replaced.Get());
				Open.Add(Replaced.Get());
			}
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
