// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVInteractionAction.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "FVInteractionTargetConfig.generated.h"

UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionTargetConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Interactable"))
	FGameplayTag Type;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Interactable"))
	TArray<TObjectPtr<UFVInteractionAction>> AvailableActions;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float FocusRadius = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float InteractionRadius = 200.f;
};
