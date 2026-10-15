#include "FVPlayerState.h"

#include "FVPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerState)

AFVPlayerController* AFVPlayerState::GetFVPlayerController() const
{
	return Cast<AFVPlayerController>(GetOwner());
}
