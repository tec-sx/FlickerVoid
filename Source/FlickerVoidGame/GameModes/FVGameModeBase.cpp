// Fill out your copyright notice in the Description page of Project Settings.

#include "FVGameModeBase.h"

#include "FlickerVoid.h"
#include "FVGameState.h"
#include "HUD/FVHUD.h"
#include "Layout/FVUILayout.h"
#include "Player/FVPlayerController.h"
#include "Player/FVPlayerState.h"
#include "Logging/MessageLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGameModeBase)

AFVGameModeBase::AFVGameModeBase()
{
	GameStateClass = AFVGameState::StaticClass();
	PlayerControllerClass = AFVPlayerController::StaticClass();
	PlayerStateClass = AFVPlayerState::StaticClass();
	DefaultPawnClass = nullptr;

	// World clock and fact database are world/game-instance subsystems driven by
	// Project Settings > FlickerVoid; add explicit bootstrapping here if needed.
}

void AFVGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	if (HUDClass == AFVHUD::StaticClass())
	{
		if (UClass* ConfiguredHUD = GetDefault<UFVUISettings>()->DefaultHUDClass.LoadSynchronous())
		{
			HUDClass = ConfiguredHUD;
		}
	}
	
	Super::InitGame(MapName, Options, ErrorMessage);

	if (!DefaultPawnClass)
	{
		const FString Message = FString::Printf(TEXT("Game mode [%s] has no Default Pawn Class, so there is no player character to spawn."), *GetNameSafe(GetClass()));

#if WITH_EDITOR
		if (GIsEditor)
		{
			UE_LOG(LogFlickerVoid, Error, TEXT("%s"), *Message);
			FMessageLog("PIE").Error(FText::FromString(Message));
			return;
		}
#endif

		UE_LOG(LogFlickerVoid, Fatal, TEXT("%s"), *Message);
	}
}
