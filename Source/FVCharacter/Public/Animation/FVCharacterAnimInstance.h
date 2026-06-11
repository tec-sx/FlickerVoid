// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "FVAnimationTypes.h"
#include "GameplayTagContainer.h"
#include "FVCharacterAnimInstance.generated.h"

class AFVCharacter;
class UFVAnimationConfigData;

/**
 * Thread-safe animation proxy for character animation.
 */
USTRUCT()
struct FLICKERVOIDCHARACTER_API FFVCharacterAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FFVCharacterAnimInstanceProxy();
	FFVCharacterAnimInstanceProxy(UAnimInstance* InAnimInstance);

protected:
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;
	virtual void Initialize(UAnimInstance* InAnimInstance) override;

public:
	FFVCharacterAnimationData CharacterData;
	TWeakObjectPtr<const UFVAnimationConfigData> Config;
	float DeltaTime;

	//~=============================================================================
	// Computed Animation State
	//~=============================================================================
	float SmoothedGroundSpeed;
	FVector SmoothedVelocity;
	FVector SmoothedAcceleration;
	bool bIsMoving;
	bool bHasAcceleration;
	bool bHasVelocity;
	float MovementDirection;
	float YawDelta;

private:
	TWeakObjectPtr<AFVCharacter> CharacterPtr;
	FTransform PreviousTransform;
	FVector PreviousVelocity;
};

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

	UFUNCTION(BlueprintPure, Category = "Animation|State")
	const FFVCharacterAnimationData& GetCharacterData() const { return Proxy.CharacterData; }

	UFUNCTION(BlueprintPure, Category = "Animation|State|Tags")
	bool HasTag(const FGameplayTag& Tag) const { return Proxy.CharacterData.CharacterTags.HasTag(Tag); }

	UFUNCTION(BlueprintPure, Category = "Animation|State|Tags")
	bool HasAllTags(const FGameplayTagContainer& Tags) const { return Proxy.CharacterData.CharacterTags.HasAll(Tags); }

	UFUNCTION(BlueprintPure, Category = "Animation|State|Tags")
	bool HasAnyTags(const FGameplayTagContainer& Tags) const { return Proxy.CharacterData.CharacterTags.HasAny(Tags); }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsMoving() const { return Proxy.bIsMoving; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsSprinting() const { return Proxy.CharacterData.IsSprinting(); }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsInAir() const { return Proxy.CharacterData.IsInAir(); }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsCrouching() const { return Proxy.CharacterData.IsCrouching(); }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool JustLanded() const { return Proxy.CharacterData.JustLanded(); }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	float GetGroundSpeed() const { return Proxy.SmoothedGroundSpeed; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	float GetMovementDirection() const { return Proxy.MovementDirection; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	FVector GetVelocity() const { return Proxy.SmoothedVelocity; }

	//~=============================================================================
	// Blueprint Extensibility (Override in BP for custom logic)
	//~=============================================================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
	void OnCharacterDataUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
	void OnTagChanged(FGameplayTag Tag, bool bAdded);

protected:
	//~=============================================================================
	// Proxy Management
	//~=============================================================================

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

private:
	FFVCharacterAnimInstanceProxy Proxy;

	UPROPERTY(Transient)
	TObjectPtr<AFVCharacter> CachedCharacter;

	FGameplayTagContainer PreviousFrameTags;

	friend struct FFVCharacterAnimInstanceProxy;
};
