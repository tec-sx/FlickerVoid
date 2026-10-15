// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FVGameModeBase.generated.h"

/**
 * Project base game mode. Wires the FlickerVoid framework defaults
 * (AFVHUD for the UI layout, FV GameState/PlayerController/PlayerState) and spawns
 * the player character from Project Settings > FlickerVoid > Game, unless a subclass
 * sets its own Default Pawn Class. Refuses to start without a player character.
 */
UCLASS(Config = Game)
class FLICKERVOID_API AFVGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFVGameModeBase();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};
