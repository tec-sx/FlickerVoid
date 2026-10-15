// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVGameModeBase.h"

#include "FVExploreGameMode.generated.h"

/** Free roam without quest restrictions, for trying things out. */
UCLASS(Config = Game)
class FLICKERVOID_API AFVExploreGameMode : public AFVGameModeBase
{
	GENERATED_BODY()
};
