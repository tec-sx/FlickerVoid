// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "FVAnimationTypes.h"
#include "GameplayTagContainer.h"
#include "FVCharacterAnimInstance.generated.h"

// Forward declarations
class AFVCharacter;
class UFVAnimationConfigData;

/**
 * Thread-safe animation proxy for character animation.
 * 
 * PERFORMANCE DESIGN:
 * - Runs on animation worker thread (parallel evaluation)
 * - Caches FFVCharacterAnimationData for safe multi-threaded access
 * - No direct character access during Update() (thread-unsafe!)
 * - All data copied in PreUpdate() on game thread, then accessed in Update()
 * 
 * WORKFLOW:
 * 1. PreUpdate (Game Thread) - Copy character data to proxy
 * 2. Update (Worker Thread) - Process animation logic using cached data
 * 3. PostUpdate (Game Thread) - Apply results back to character if needed
 */
USTRUCT()
struct FLICKERVOIDCHARACTER_API FFVCharacterAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FFVCharacterAnimInstanceProxy();
	FFVCharacterAnimInstanceProxy(UAnimInstance* InAnimInstance);

	//~=============================================================================
	// FAnimInstanceProxy Interface (Thread-Safe Lifecycle)
	//~=============================================================================

protected:
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;
	virtual void Initialize(UAnimInstance* InAnimInstance) override;

public:

	//~=============================================================================
	// Cached Character Data (Updated PreUpdate, Read-Only in Update)
	//~=============================================================================

	/** Complete character state snapshot (copied each frame on game thread) */
	FFVCharacterAnimationData CharacterData;

	/** Configuration data (set once on initialize, read-only thereafter) */
	TWeakObjectPtr<const UFVAnimationConfigData> Config;

	/** Delta time for this frame */
	float DeltaTime;

	//~=============================================================================
	// Computed Animation State (Calculated in Update, thread-safe)
	//~=============================================================================

	/** Smoothed ground speed (interpolated) */
	float SmoothedGroundSpeed;

	/** Smoothed velocity (interpolated for animation) */
	FVector SmoothedVelocity;

	/** Smoothed acceleration (interpolated) */
	FVector SmoothedAcceleration;

	/** Is character moving? (speed above threshold) */
	bool bIsMoving;

	/** Has any acceleration input? */
	bool bHasAcceleration;

	/** Has any velocity? */
	bool bHasVelocity;

	/** Direction of movement relative to rotation (0-360 degrees) */
	float MovementDirection;

	/** Yaw delta for current frame (for turn-in-place) */
	float YawDelta;

private:
	/** Character reference (ONLY access in PreUpdate - game thread only!) */
	TWeakObjectPtr<AFVCharacter> CharacterPtr;

	/** Previous frame transform (for delta calculations) */
	FTransform PreviousTransform;

	/** Previous frame velocity */
	FVector PreviousVelocity;
};

/**
 * Tag-based, data-driven character animation instance.
 * 
 * ARCHITECTURE:
 * - NO ENUMS - All state queries use gameplay tags
 * - Data-driven configuration via UFVAnimationConfigData
 * - Performance-optimized with proper FAnimInstanceProxy usage
 * - Extensible in Blueprint for archetype-specific animation logic
 * 
 * USAGE:
 * 1. Assign AnimationConfig data asset in Blueprint
 * 2. Query tags via CharacterData.CharacterTags in Animation BP
 * 3. Extend in Blueprint for custom animation logic per archetype
 * 
 * PERFORMANCE:
 * - Multi-threaded animation evaluation (proxy runs on worker thread)
 * - Minimal game thread overhead (just data copy in PreUpdate)
 * - Tag queries cached and fast (binary search)
 * - Smoothed values pre-calculated for animation use
 */
UCLASS(Blueprintable, BlueprintType)
class FLICKERVOIDCHARACTER_API UFVCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFVCharacterAnimInstance(const FObjectInitializer& ObjectInitializer);

	//~=============================================================================
	// UAnimInstance Interface
	//~=============================================================================

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeBeginPlay() override;

	//~=============================================================================
	// Configuration
	//~=============================================================================

	/** Animation configuration data asset (assign per archetype) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TObjectPtr<UFVAnimationConfigData> AnimationConfig;

	//~=============================================================================
	// Character State Accessors (Tag-Based Queries)
	//~=============================================================================

	/** Get current character animation data (includes all tags) */
	UFUNCTION(BlueprintPure, Category = "Animation|State")
	const FFVCharacterAnimationData& GetCharacterData() const { return Proxy.CharacterData; }

	/** Check if character has specific tag */
	UFUNCTION(BlueprintPure, Category = "Animation|State|Tags")
	bool HasTag(const FGameplayTag& Tag) const { return Proxy.CharacterData.CharacterTags.HasTag(Tag); }

	/** Check if character has all tags */
	UFUNCTION(BlueprintPure, Category = "Animation|State|Tags")
	bool HasAllTags(const FGameplayTagContainer& Tags) const { return Proxy.CharacterData.CharacterTags.HasAll(Tags); }

	/** Check if character has any tags */
	UFUNCTION(BlueprintPure, Category = "Animation|State|Tags")
	bool HasAnyTags(const FGameplayTagContainer& Tags) const { return Proxy.CharacterData.CharacterTags.HasAny(Tags); }

	//~=============================================================================
	// Common Animation Queries (Convenience Methods)
	//~=============================================================================

	/** Is character moving? */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsMoving() const { return Proxy.bIsMoving; }

	/** Is character sprinting? */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsSprinting() const { return Proxy.CharacterData.IsSprinting(); }

	/** Is character in air? */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsInAir() const { return Proxy.CharacterData.IsInAir(); }

	/** Is character crouching? */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsCrouching() const { return Proxy.CharacterData.IsCrouching(); }

	/** Just landed? */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool JustLanded() const { return Proxy.CharacterData.JustLanded(); }

	/** Get ground speed (smoothed) */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	float GetGroundSpeed() const { return Proxy.SmoothedGroundSpeed; }

	/** Get movement direction (0-360 degrees relative to rotation) */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	float GetMovementDirection() const { return Proxy.MovementDirection; }

	/** Get velocity (smoothed) */
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	FVector GetVelocity() const { return Proxy.SmoothedVelocity; }

	//~=============================================================================
	// Blueprint Extensibility (Override in BP for custom logic)
	//~=============================================================================

	/** Called after character data updated (extend in Blueprint for custom processing) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
	void OnCharacterDataUpdated();

	/** Called when specific tag added/removed (extend in Blueprint for reactive logic) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
	void OnTagChanged(FGameplayTag Tag, bool bAdded);

protected:
	//~=============================================================================
	// Proxy Management
	//~=============================================================================

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

private:
	/** The animation proxy (handles thread-safe updates) */
	FFVCharacterAnimInstanceProxy Proxy;

	/** Cached character reference */
	UPROPERTY(Transient)
	TObjectPtr<AFVCharacter> CachedCharacter;

	/** Previous frame tags (for change detection) */
	FGameplayTagContainer PreviousFrameTags;

	friend struct FFVCharacterAnimInstanceProxy;
};
