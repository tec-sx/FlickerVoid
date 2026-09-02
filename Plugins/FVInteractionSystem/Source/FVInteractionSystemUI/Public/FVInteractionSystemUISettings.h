// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVInteractionSystemUISettings.generated.h"

struct FGameplayTag;
struct FInteractionSlotStyle;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Interaction System UI"))
class FVINTERACTIONSYSTEMUI_API UFVInteractionSystemUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFVInteractionSystemUISettings(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(Config, EditAnywhere, Category = "Style", meta = (RequiredAssetDataTags = "RowStructure=/Script/FVInteractionSystemUI.InteractionSlotStyle"))
	TSoftObjectPtr<UDataTable> InteractionSlotStyles;
	
	UFUNCTION(BlueprintCallable)
	FInteractionSlotStyle GetSlotStyle(const FGameplayTag ActionTag) const;
};
