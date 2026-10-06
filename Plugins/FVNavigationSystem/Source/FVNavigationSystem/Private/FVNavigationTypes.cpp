#include "FVNavigationTypes.h"

#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigationTypes)

UFVNavigationSettings::UFVNavigationSettings()
{
	CaptureFolder.Path = TEXT("/Game/Navigation/MapCaptures");
	CaptureIgnoredClasses.Add(APawn::StaticClass());
	CaptureIgnoredActorTags.Add(TEXT("MapCaptureIgnore"));
}
