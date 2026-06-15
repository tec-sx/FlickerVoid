// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVInteractionInstigatorConfig.generated.h"

/**
 * 
 */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionInstigatorConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DetectionRadius = 350.f;

	// Cosine of the half-angle of the cone in which an interactable can be focused
	// (0 = 90°, 0.5 = ~60°, 0.707 = 45°). Interactables outside this cone are ignored.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "1"))
	float FocusConeCosine = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float FocusUpdateInterval = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TEnumAsByte<EObjectTypeQuery>> DetectionObjectTypes;
};
