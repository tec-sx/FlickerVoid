#include "Scales/FVScaleDefinition.h"

#include "Facts/FVFactSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScaleDefinition)

#define LOCTEXT_NAMESPACE "FVScaleDefinition"

const FFVScaleBand* UFVScaleDefinition::FindBand(const int32 Value) const
{
	const FFVScaleBand* Result = nullptr;
	for (const FFVScaleBand& Band : Bands)
	{
		if (Value >= Band.Threshold && (Result == nullptr || Band.Threshold > Result->Threshold))
		{
			Result = &Band;
		}
	}
	return Result;
}

#if WITH_EDITOR
EDataValidationResult UFVScaleDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	const FFVFactDefinition* Fact = GetDefault<UFVFactSettings>()->FindDefinition(Id);
	if (Id.IsValid() && (Fact == nullptr || !Fact->bClamp))
	{
		Context.AddWarning(FText::Format(LOCTEXT("Unbounded", "{0} has no range: give its fact {1} a Min and Max in the Facts settings."),
			FText::FromName(GetFName()), FText::FromName(Id.GetTagName())));
	}

	for (const FFVScaleLink& Link : Links)
	{
		if (Link.Scale == this)
		{
			Context.AddError(FText::Format(LOCTEXT("SelfLink", "{0} links to itself."), FText::FromName(GetFName())));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
