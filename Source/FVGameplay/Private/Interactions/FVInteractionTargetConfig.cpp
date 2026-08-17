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

	if (!InteractionSet)
	{
		Context.AddError(LOCTEXT("NullInteractionSet", "InteractionSet is not set."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

#endif
