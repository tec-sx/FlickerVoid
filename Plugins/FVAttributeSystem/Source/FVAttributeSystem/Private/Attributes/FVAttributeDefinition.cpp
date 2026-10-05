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

	if (Min > Max)
	{
		Context.AddError(LOCTEXT("BadRange", "Min is greater than Max."));
		Result = EDataValidationResult::Invalid;
	}
	else if (DefaultValue < Min || DefaultValue > Max)
	{
		Context.AddWarning(LOCTEXT("DefaultOutOfRange", "DefaultValue is outside [Min, Max] and will be clamped."));
	}

	return Result;
}
#endif

void UFVAttributeSetDefinition::Collect(TArray<FFVAttributeValue>& OutValues) const
{
	for (const TObjectPtr<const UFVAttributeSetDefinition>& Parent : Parents)
	{
		if (Parent != nullptr && Parent != this)
		{
			Parent->Collect(OutValues);
		}
	}

	for (const FFVAttributeValue& Value : Attributes)
	{
		if (Value.Attribute == nullptr)
		{
			continue;
		}

		if (FFVAttributeValue* Existing = OutValues.FindByPredicate([&Value](const FFVAttributeValue& Entry) { return Entry.Attribute == Value.Attribute; }))
		{
			Existing->Value = Value.Value;
		}
		else
		{
			OutValues.Add(Value);
		}
	}
}

#undef LOCTEXT_NAMESPACE
