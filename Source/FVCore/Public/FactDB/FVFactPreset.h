// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "FVFactPreset.generated.h"

UCLASS()
class FLICKERVOIDCORE_API UFVFactPreset final : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Fact", meta=(Categories="Fact", ForceInlineRow))
	TMap<FGameplayTag, int32> PresetValues;
};
