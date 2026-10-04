// Fill out your copyright notice in the Description page of Project Settings.

#include "FVGameModeBase.h"

#include "Character/FVCharacter.h"
#include "FVGameState.h"
#include "HUD/FVHUD.h"
#include "Layout/FVUILayout.h"
#include "Player/FVPlayerController.h"
#include "Player/FVPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGameModeBase)

AFVGameModeBase::AFVGameModeBase()
{
	GameStateClass = AFVGameState::StaticClass();
	PlayerControllerClass = AFVPlayerController::StaticClass();
	PlayerStateClass = AFVPlayerState::StaticClass();
	DefaultPawnClass = AFVCharacter::StaticClass();

	// The HUD creates the FV UI layout for each local player.
	UClass* ConfiguredHUD = GetDefault<UFVUISettings>()->DefaultHUDClass.LoadSynchronous();
	HUDClass = ConfiguredHUD ? ConfiguredHUD : AFVHUD::StaticClass();

	// World clock and fact database are world/game-instance subsystems driven by
	// Project Settings > FlickerVoid; add explicit bootstrapping here if needed.
}
