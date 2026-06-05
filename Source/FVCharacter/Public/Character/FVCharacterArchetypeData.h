// Copyright FlickerVoid. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FVCharacterArchetypeData.generated.h"

class UFVLocomotionStyleData;
class UFVMovementHandlerData;

UCLASS(BlueprintType, Const, meta = (DisplayName = "Character Archetype Data"))
class FLICKERVOIDCHARACTER_API UFVCharacterArchetypeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVCharacterArchetypeData();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	//~=============================================================================
	// Identification
	//~=============================================================================

	/** Unique identifier for this archetype */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ArchetypeId;

	/** Display name */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	//~=============================================================================
	// Initial Tags (Applied on character creation/possession)
	//~=============================================================================

	/**
	 * Permanent tags for this archetype
	 * e.g., Character.Archetype.Soldier, Character.Trait.Aggressive
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags|Permanent")
	FGameplayTagContainer PermanentTags;

	/**
	 * Initial tags (can be changed during gameplay)
	 * e.g., Character.Mood.Calm, Movement.Gait.Walking
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags|Initial")
	FGameplayTagContainer InitialTags;

	/**
	 * Tags that should never be added to this archetype
	 * e.g., block Movement.Gait.Sprinting for injured characters
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags|Restrictions")
	FGameplayTagContainer BlockedTags;

	//~=============================================================================
	// Movement Configuration
	//~=============================================================================

	/** Movement handler configuration for this archetype */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UFVMovementHandlerData> MovementConfig;

	/** Base movement speed modifier (1.0 = normal, 0.5 = half speed, 1.5 = 150% speed) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MovementSpeedModifier = 1.0f;

	/** Jump velocity modifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float JumpHeightModifier = 1.0f;

	/** Acceleration modifier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float AccelerationModifier = 1.0f;

	//~=============================================================================
	// Animation Configuration
	//~=============================================================================

	/** Locomotion animation style for this archetype */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UFVLocomotionStyleData> LocomotionStyle;

	/** Animation tags that modify animation behavior (e.g., Animation.Style.Feminine) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	FGameplayTagContainer AnimationStyleTags;

	//~=============================================================================
	// Gameplay Properties
	//~=============================================================================

	/** Base health value (-1 = use default) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "-1"))
	float BaseHealth = -1.0f;

	/** Base stamina value (-1 = use default) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "-1"))
	float BaseStamina = -1.0f;

	/** Can this archetype sprint? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Abilities")
	bool bCanSprint = true;

	/** Can this archetype crouch? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Abilities")
	bool bCanCrouch = true;

	/** Can this archetype jump? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Abilities")
	bool bCanJump = true;

	/** Can this archetype perform traversal actions? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Abilities")
	bool bCanTraverse = true;

	//~=============================================================================
	// Visual Properties
	//~=============================================================================

	/** Default mesh to use (optional override) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<USkeletalMesh> DefaultMesh;

	/** Material overrides */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;

	//~=============================================================================
	// Helper Functions
	//~=============================================================================

	/** Get all tags that should be applied (permanent + initial) */
	UFUNCTION(BlueprintCallable, Category = "Character Archetype")
	FGameplayTagContainer GetAllInitialTags() const;

	/** Should this archetype block a specific tag? */
	UFUNCTION(BlueprintCallable, Category = "Character Archetype")
	bool IsTagBlocked(const FGameplayTag& Tag) const;

	/** Apply ability restrictions based on archetype settings */
	UFUNCTION(BlueprintCallable, Category = "Character Archetype")
	FGameplayTagContainer GetAbilityRestrictionTags() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

