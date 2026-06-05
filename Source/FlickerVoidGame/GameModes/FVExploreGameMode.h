// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FVExploreGameMode.generated.h"

#define UE_API FLICKERVOID_API

class UFVPawnData;
class APawn;
class AActor;
class AController;
class APlayerController;

UCLASS(MinimalAPI, Config = Game)
class AFVExploreGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UE_API AFVExploreGameMode();

	UE_API const UFVPawnData* GetPawnDataForController(const AController* InController) const;

	//~AGameModeBase interface
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	//~End of AGameModeBase interface
};

#undef UE_API