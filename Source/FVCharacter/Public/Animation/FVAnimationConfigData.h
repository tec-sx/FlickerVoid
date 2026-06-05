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
	UFVAnimationConfigData();

	//~=============================================================================
	// Movement Thresholds
	//~=============================================================================

	/** Speed threshold to be considered "moving" (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float MovingSpeedThreshold = 10.0f;

	/** Speed threshold for walking gait (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float WalkingSpeedThreshold = 150.0f;

	/** Speed threshold for jogging gait (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float JoggingSpeedThreshold = 300.0f;

	/** Speed threshold for sprinting gait (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
	float SprintingSpeedThreshold = 500.0f;

	//~=============================================================================
	// Rotation & Turn Settings
	//~=============================================================================

	/** Maximum turn rate for smooth rotation (deg/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float MaxTurnRate = 360.0f;

	/** Angle threshold to trigger turn-in-place animation (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float TurnInPlaceThreshold = 45.0f;

	/** Time before starting turn-in-place after stopping (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float TurnInPlaceDelay = 0.5f;

	//~=============================================================================
	// Blend Times
	//~=============================================================================

	/** Blend time for gait changes (walk <-> jog <-> sprint) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Times")
	float GaitBlendTime = 0.2f;

	/** Blend time for stance changes (stand <-> crouch) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Times")
	float StanceBlendTime = 0.3f;

	/** Blend time for rotation mode changes (strafe <-> orient to movement) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Times")
	float RotationModeBlendTime = 0.4f;

	//~=============================================================================
	// Landing Settings
	//~=============================================================================

	/** Minimum landing velocity to trigger landing animation (Z-axis, cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float LandingVelocityThreshold = 300.0f;

	/** Time window to check "just landed" state (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing")
	float JustLandedDuration = 0.3f;

	//~=============================================================================
	// Interpolation Speeds
	//~=============================================================================

	/** Interpolation speed for velocity smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation")
	float VelocityInterpSpeed = 12.0f;

	/** Interpolation speed for acceleration smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation")
	float AccelerationInterpSpeed = 8.0f;

	//~=============================================================================
	// Debug Settings
	//~=============================================================================

	/** Enable debug visualization in viewport */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableDebugDraw = false;

	/** Enable verbose logging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableVerboseLogging = false;
};


