#include "Social/FVFactionDefinition.h"

#include "Scales/FVScaleDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactionDefinition)

#define LOCTEXT_NAMESPACE "FVFactionDefinition"

EFVAttitude UFVFactionDefinition::GetStandingAttitude(const int32 Value) const
{
	const FFVScaleBand* Band = Standing ? Standing->FindBand(Value) : nullptr;
	const EFVAttitude* Attitude = Band ? BandAttitudes.Find(Band->Tag) : nullptr;
	return Attitude ? *Attitude : EFVAttitude::Neutral;
}

#if WITH_EDITOR
EDataValidationResult UFVFactionDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!Standing)
	{
		Context.AddError(FText::Format(LOCTEXT("NoStanding", "{0} has no Standing scale."), FText::FromName(GetFName())));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
