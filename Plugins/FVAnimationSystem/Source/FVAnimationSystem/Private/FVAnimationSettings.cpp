#include "FVAnimationSettings.h"
#include "FVCoreNames.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAnimationSettings)

UFVAnimationSettings::UFVAnimationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = FV::Names::SettingsCategory;
	SectionName = "Animation System";
	SidePairs.Add(FFVSidePair());
}
