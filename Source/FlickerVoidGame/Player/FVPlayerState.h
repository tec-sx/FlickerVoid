#pragma once

#include "GameFramework/PlayerState.h"

#include "FVPlayerState.generated.h"

class AFVPlayerController;

UCLASS(Config = Game)
class FLICKERVOID_API AFVPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|PlayerState")
	AFVPlayerController* GetFVPlayerController() const;
};
