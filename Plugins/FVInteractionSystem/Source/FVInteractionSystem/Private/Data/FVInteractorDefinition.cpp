#include "Data/FVInteractorDefinition.h"

#include "Data/FVInteractorModeDefinition.h"
#include "FVInteractionSystemSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorDefinition)

#define LOCTEXT_NAMESPACE "FVInteractorDefinition"

UFVInteractorDefinition::UFVInteractorDefinition()
	: InteractorTag(UFVInteractionSystemSettings::Get().InteractorDefaultSettings.InteractorTag)
{
}

const UFVInteractorModeDefinition* UFVInteractorDefinition::FindMode(const FGameplayTag& ModeTag) const
{
	if (DefaultMode && DefaultMode->ModeTag.MatchesTagExact(ModeTag))
	{
		return DefaultMode;
	}

	for (const UFVInteractorModeDefinition* Mode : Modes)
	{
		if (Mode && Mode->ModeTag.MatchesTagExact(ModeTag))
		{
			return Mode;
		}
	}

	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UFVInteractorDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!DefaultMode)
	{
		Context.AddError(LOCTEXT("NoDefaultMode", "Interactor definition requires a DefaultMode."));
		Result = EDataValidationResult::Invalid;
	}

	TSet<FGameplayTag> SeenTags;
	if (DefaultMode)
	{
		SeenTags.Add(DefaultMode->ModeTag);
	}

	for (const UFVInteractorModeDefinition* Mode : Modes)
	{
		if (!Mode)
		{
			Context.AddWarning(LOCTEXT("NullMode", "Interactor definition has an empty mode entry."));
			continue;
		}

		if (SeenTags.Contains(Mode->ModeTag))
		{
			Context.AddError(FText::Format(LOCTEXT("DuplicateMode", "Interactor definition declares mode '{0}' more than once."), FText::FromString(Mode->ModeTag.ToString())));
			Result = EDataValidationResult::Invalid;
		}
		SeenTags.Add(Mode->ModeTag);
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
