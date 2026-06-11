// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVAnimationConfigData.generated.h"

/**
 * Configuration data asset for character animation system.
 * 
 * USAGE:
 * - Create one per character archetype (Player, Heavy, Agile, etc.)
 * - Assign to AnimInstance via Blueprint or Character setup
 * - Contains animation thresholds, turn rates, blend times
 * 
 * EXTENSIBILITY:
 * - Subclass in Blueprint to add archetype-specific settings
 * - Query different settings based on character tags
 */
UCLASS(BlueprintType)
class FLICKERVOIDCHARACTER_API UFVAnimationConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~=============================================================================
	// Movement Thresholds
	//~=============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float MovingSpeedThreshold = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float WalkingSpeedThreshold = 150.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float JoggingSpeedThreshold = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float SprintingSpeedThreshold = 500.0f;

	//~=============================================================================
	// Rotation & Turn Settings
	//~=============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float MaxTurnRate = 360.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float TurnInPlaceThreshold = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float TurnInPlaceDelay = 0.5f;

	//~=============================================================================
	// Blend Times
	//~=============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Times")
	float GaitBlendTime = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Times")
	float StanceBlendTime = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Times")
	float RotationModeBlendTime = 0.4f;

	//~=============================================================================
	// Landing Settings
	//~=============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingVelocityThreshold = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float JustLandedDuration = 0.3f;

	//~=============================================================================
	// Interpolation Speeds
	//~=============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation")
	float VelocityInterpSpeed = 12.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation")
	float AccelerationInterpSpeed = 8.0f;

	//~=============================================================================
	// Debug Settings
	//~=============================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableDebugDraw = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableVerboseLogging = false;
};


