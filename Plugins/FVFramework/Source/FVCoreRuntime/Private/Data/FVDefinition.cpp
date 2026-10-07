#include "Data/FVDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "FVDefinition"

FPrimaryAssetId UFVDefinition::GetPrimaryAssetId() const
{
	const UClass* NativeClass = GetClass();
	while (NativeClass && !NativeClass->HasAnyClassFlags(CLASS_Native))
	{
		NativeClass = NativeClass->GetSuperClass();
	}
	return FPrimaryAssetId(NativeClass ? NativeClass->GetFName() : GetClass()->GetFName(), GetFName());
}

#if WITH_EDITOR
EDataValidationResult UFVDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (RequiresId() && !Id.IsValid())
	{
		Context.AddError(FText::Format(LOCTEXT("MissingId", "{0} has no Id tag; its state is stored in the fact it names."), FText::FromName(GetFName())));
		Result = EDataValidationResult::Invalid;
	}

	if (Display.Name.IsEmpty())
	{
		Context.AddWarning(FText::Format(LOCTEXT("MissingName", "{0} has no display name."), FText::FromName(GetFName())));
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
