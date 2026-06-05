// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "InteractionSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game)
class FLICKERVOID_API UInteractionSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = Interaction, meta = (ConsoleVariable = "FVoid.Interaction.Debug.DrawInteractionTarget"))
	bool bDebugDrawInteractionTarget = false;
};
