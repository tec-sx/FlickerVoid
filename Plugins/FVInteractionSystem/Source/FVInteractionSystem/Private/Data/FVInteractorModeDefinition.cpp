#include "Data/FVInteractorModeDefinition.h"

#include "FVInteractionSystemSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorModeDefinition)

#define LOCTEXT_NAMESPACE "FVInteractorModeDefinition"

UFVInteractorModeDefinition::UFVInteractorModeDefinition()
{
	Detection.CollisionChannel = UFVInteractionSystemSettings::Get().InteractorDefaultSettings.CollisionChannel;
}

#if WITH_EDITOR
EDataValidationResult UFVInteractorModeDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!ModeTag.IsValid())
	{
		Context.AddError(LOCTEXT("NoModeTag", "Interactor mode has no ModeTag."));
		Result = EDataValidationResult::Invalid;
	}

	if (Detection.TraceRadius < 0.f || Detection.TraceRange <= 0.f || Detection.TickInterval <= 0.f)
	{
		Context.AddError(LOCTEXT("BadDetection", "Interactor mode requires TraceRadius >= 0, TraceRange > 0 and TickInterval > 0."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
