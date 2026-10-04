#include "FVInteractionSystemSettings.h"
#include "FVCoreNames.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionSystemSettings)

UFVInteractionSystemSettings::UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = FV::Names::SettingsCategory;
	SectionName = "Interaction System";
}


