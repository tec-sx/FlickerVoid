// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "FVCharacterTypes.h"
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
	FFVCharacterRuntimeState CharacterRuntimeState;
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
	
	UFUNCTION(BlueprintPure, Category = "Animation|State")
	const FFVCharacterRuntimeState& GetCharacterRuntimeState() const { return Proxy.CharacterRuntimeState; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool IsMoving() const { return Proxy.bIsMoving; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	EFVGait GetGait() const { return Proxy.CharacterRuntimeState.Gait; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	EFVStance GetStance() const { return Proxy.CharacterRuntimeState.Stance; }
	
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	EFVMovementMode GetMovementMode() const { return Proxy.CharacterRuntimeState.MovementMode; }
	
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	bool JustLanded() const { return Proxy.CharacterRuntimeState.bJustLanded; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	float GetGroundSpeed() const { return Proxy.SmoothedGroundSpeed; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	float GetMovementDirection() const { return Proxy.MovementDirection; }

	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	FVector GetVelocity() const { return Proxy.SmoothedVelocity; }
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
	void OnCharacterDataUpdated();

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
	
	friend struct FFVCharacterAnimInstanceProxy;
};
