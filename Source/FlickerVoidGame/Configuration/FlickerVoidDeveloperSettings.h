#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "FlickerVoidDeveloperSettings.generated.h"

class AFVPlayerCharacter;

/** Game settings for FlickerVoid (Project Settings > FlickerVoid > Game). */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game"))
class FLICKERVOID_API UFlickerVoidDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

    UPROPERTY(Config, EditAnywhere, Category = "Player", meta = (ToolTip = "Character the game modes spawn for the player unless a game mode sets its own Default Pawn Class. The game doesn't start without one."))
    TSoftClassPtr<AFVPlayerCharacter> DefaultPlayerCharacterClass;
};
