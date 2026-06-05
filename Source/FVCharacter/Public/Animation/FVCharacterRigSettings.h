// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVCharacterRigSettings.generated.h"

/**
 * UFVCharacterRigSettings
 * 
 * Primary data asset that defines Control Rig parameters for procedural animation.
 * Assign variations of this to skeletal meshes to customize animation style.
 * 
 * Examples:
 * - Feminine walk: Higher HipSwingAmount (1.3-1.5), lower SpineCompensation
 * - Military walk: Lower HipSwingAmount (0.3-0.5), higher SpineCompensation
 * - Neutral: Default values (1.0)
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "FV Character Rig Settings", ShortTooltip = "Control Rig parameters for procedural animation"))
class FLICKERVOIDCHARACTER_API UFVCharacterRigSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVCharacterRigSettings();

	// ========================================================================
	// ASSET IDENTIFICATION
	// ========================================================================

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Unique identifier for this rig settings preset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName SettingsId;

	/** Display name for the editor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	// ========================================================================
	// HIP SWING SETTINGS
	// ========================================================================

	/**
	 * Master multiplier for hip swing motion in walk cycles.
	 * 1.0 = original animation
	 * 0.0 = no hip swing (robotic)
	 * 1.5 = exaggerated swing (feminine/sassy)
	 * 0.5 = restrained swing (military/masculine)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hip Swing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float HipSwingAmount = 1.0f;

	/**
	 * Multiplier for lateral (side-to-side) hip sway.
	 * Stacks with HipSwingAmount.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hip Swing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LateralSwingMultiplier = 1.0f;

	/**
	 * Multiplier for rotational hip swing (twist on vertical axis).
	 * Stacks with HipSwingAmount.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hip Swing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float RotationalSwingMultiplier = 1.0f;

	/**
	 * Multiplier for vertical hip bob during walking.
	 * Affects up/down motion of the pelvis.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hip Swing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VerticalBobAmount = 1.0f;

	/**
	 * How much the spine counter-rotates against hip movement.
	 * 0.0 = no compensation (upper body follows hips)
	 * 1.0 = full compensation (upper body stays stable)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hip Swing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpineCompensation = 0.5f;

	// ========================================================================
	// BONE NAMES (Skeleton-specific)
	// ========================================================================

	/** Name of the pelvis/hip bone */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skeleton")
	FName PelvisBoneName = FName(TEXT("pelvis"));

	/** Name of the first spine bone for counter-rotation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skeleton")
	FName SpineBoneName = FName(TEXT("spine_01"));
};
