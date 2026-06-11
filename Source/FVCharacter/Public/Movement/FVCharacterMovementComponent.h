// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FVCharacterMovementTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "Movement/FVMovementHandlerData.h"
#include "FVCharacterMovementComponent.generated.h"

// Forward declarations
class UFVMovementHandlerBase;
class AFVCharacter;
struct FFVMovementHandlerInfo;

/**
 * Data-driven character movement component
 * Uses UFVMovementHandlerDataAsset to configure movement handlers
 * Supports gameplay tag-based activation similar to GAS abilities
 */
UCLASS(Config = Game, meta = (BlueprintSpawnableComponent))
class FLICKERVOIDCHARACTER_API UFVCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UFVCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Configuration")
	TObjectPtr<UFVMovementHandlerData> MovementConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TArray<TObjectPtr<UFVMovementHandlerBase>> RegisteredHandlers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UFVMovementHandlerBase> CurrentHandler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintAngleThreshold = 50.f;

	UFUNCTION(BlueprintCallable, Category = "Movement|Extension")
	bool RegisterHandler(const FFVMovementHandlerInfo& HandlerInfo);
	
	UFUNCTION(BlueprintPure, Category = "Movement")
	UFVMovementHandlerBase* GetCurrentMovementHandler() const { return CurrentHandler; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	TArray<UFVMovementHandlerBase*> GetAllHandlers() const;
	
	UFUNCTION(BlueprintPure, Category = "Movement")
	AFVCharacter* GetFVCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Movement|Configuration")
	void ReloadConfiguration();

private:
	AFVCharacter* Character;
	FGameplayTagContainer ActiveTags;
	bool bIsInitialized;

	void InitializeHandlers();
	void TransitionToHandler(UFVMovementHandlerBase* NewHandler);
	UFVMovementHandlerBase* SelectHandler() const;


public:
//~=============================================================================
// Debug
//~=============================================================================

#if WITH_EDITOR
/** Validate the configuration */
virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

