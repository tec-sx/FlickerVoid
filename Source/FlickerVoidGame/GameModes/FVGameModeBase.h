// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FVGameModeBase.generated.h"

/**
 * Project base game mode. Wires the FlickerVoid framework defaults
 * (AFVHUD for the UI layout, FV GameState/PlayerController/PlayerState).
 * Serves as a reference for using the FV plugins in any game mode.
 * Subclasses set the player character as Default Pawn Class; the game refuses to start without one.
 */
UCLASS(Config = Game)
class FLICKERVOID_API AFVGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFVGameModeBase();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};
