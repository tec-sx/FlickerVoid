#include "Attributes/FVAttributeDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeDefinition)

#define LOCTEXT_NAMESPACE "FVAttributeDefinition"

float UFVAttributeDefinition::Clamp(float Value) const
{
	const float Clamped = FMath::Clamp(Value, Min, Max);
	return bInteger ? FMath::RoundToFloat(Clamped) : Clamped;
}

#if WITH_EDITOR
EDataValidationResult UFVAttributeDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!Attribute.IsValid())
	{
		Context.AddError(LOCTEXT("NoAttribute", "No attribute selected."));
		Result = EDataValidationResult::Invalid;
	}

	if (Min > Max)
	{
		Context.AddError(LOCTEXT("BadRange", "Min is greater than Max."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
