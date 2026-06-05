// Fill out your copyright notice in the Description page of Project Settings.


#include "FVExploreGameMode.h"
#include "FVGameState.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Systems/FVAssetManager.h"
#include "Player/FVPlayerController.h"
#include "Player/FVPlayerState.h"
#include "Character/FVCharacter.h"
#include "FVHUD.h"
//#include "Character/LyraPawnExtensionComponent.h"
#include "Character/FVPawnData.h"
//#include "GameModes/LyraWorldSettings.h"
#include "Kismet/GameplayStatics.h"
//#include "Development/LyraDeveloperSettings.h"
//#include "Player/LyraPlayerSpawningManagerComponent.h"
//#include "CommonUserSubsystem.h"
//#include "CommonSessionSubsystem.h"
#include "TimerManager.h"
#include "GameMapsSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVExploreGameMode)

AFVExploreGameMode::AFVExploreGameMode()
{
	GameStateClass = AFVGameState::StaticClass();
	PlayerControllerClass = AFVPlayerController::StaticClass();
	PlayerStateClass = AFVPlayerState::StaticClass();
	DefaultPawnClass = AFVCharacter::StaticClass();
	HUDClass = AFVHUD::StaticClass();
}

const UFVPawnData* AFVExploreGameMode::GetPawnDataForController(const AController* InController) const
{
	// See if pawn data is already set on the player state
	if (InController != nullptr)
	{
		if (const AFVPlayerState* FVPS = InController->GetPlayerState<AFVPlayerState>())
		{
			if (const UFVPawnData* PawnData = FVPS->GetPawnData<UFVPawnData>())
			{
				return PawnData;
			}
		}
	}

	// TODO: In Lyra, we check experience for Pawn data, but since we don't use experiences yet, we'll skip that for now.

	return UFVAssetManager::Get().GetDefaultPawnData();
}

void AFVExploreGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

UClass* AFVExploreGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UFVPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* AFVExploreGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (AFVPlayerState* PS = NewPlayer->GetPlayerState<AFVPlayerState>())
			{
				if (const UFVPawnData* PawnData = UFVAssetManager::Get().GetDefaultPawnData())
				{
					PS->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);

			return SpawnedPawn;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}
