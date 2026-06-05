// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	//~=============================================================================
	// Configuration
	//~=============================================================================

	/** Movement handler configuration data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Configuration")
	TObjectPtr<UFVMovementHandlerData> MovementConfig;

	/** Maximum angle (degrees) from forward direction to allow sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Configuration")
	float SprintAngleThreshold = 50.f;

	//~=============================================================================
	// Movement Handler Control (Tag-Based)
	//~=============================================================================

	/**
	 * Try to activate handler with specific activation tags
	 * @param ActivationTags Tags required to activate handler
	 * @param bForce Force activation regardless of rules
	 * @return True if handler was activated
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool TryActivateHandlerByTags(const FGameplayTagContainer& ActivationTags);

	/**
	 * Find and activate the best handler for current character state
	 * Checks all handlers and activates highest priority one that can activate
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void UpdateActiveHandler();

	/** Get the current active movement handler */
	UFUNCTION(BlueprintPure, Category = "Movement")
	UFVMovementHandlerBase* GetCurrentMovementHandler() const { return CurrentMovementHandler; }

	/** Get a handler by its activation tags */
	UFUNCTION(BlueprintPure, Category = "Movement")
	UFVMovementHandlerBase* GetHandlerByTags(const FGameplayTagContainer& ActivationTags) const;

	/** Check if a handler with specific tags can activate */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool CanActivateHandlerByTags(const FGameplayTagContainer& ActivationTags) const;

	/** Get all registered handlers */
	UFUNCTION(BlueprintPure, Category = "Movement")
	TArray<UFVMovementHandlerBase*> GetAllHandlers() const;

	//~=============================================================================
	// Handler Registration (For Runtime Extension)
	//~=============================================================================

	/**
	 * Register a new handler at runtime
	 * Allows dynamic addition of movement behaviors via Blueprint/Script
	 * @param HandlerInfo Configuration for the handler
	 * @return True if handler was registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement|Extension")
	bool RegisterHandler(const FFVMovementHandlerInfo& HandlerInfo);

	/**
	 * Unregister a handler by its activation tags
	 * @param ActivationTags Tags identifying the handler to remove
	 * @return True if handler was found and removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement|Extension")
	bool UnregisterHandlerByTags(const FGameplayTagContainer& ActivationTags);

	//~=============================================================================
	// Helper Functions
	//~=============================================================================

	/** Get the FVCharacter owner */
	UFUNCTION(BlueprintPure, Category = "Movement")
	AFVCharacter* GetFVCharacter() const;

	/** Reload configuration from the data asset */
	UFUNCTION(BlueprintCallable, Category = "Movement|Configuration")
	void ReloadConfiguration();

protected:
	//~=============================================================================
	// UActorComponent Interface
	//~=============================================================================

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//~=============================================================================
	// UCharacterMovementComponent Interface
	//~=============================================================================

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void InitializeHandlers();
	bool CanTransition() const;
	void TransitionToHandler(UFVMovementHandlerBase* NewHandler);

	UFVMovementHandlerBase* FindBestHandler() const;

protected:
	/**
	 * All registered movement handlers
	 * Stored as array for iteration, handlers matched by activation tags
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UFVMovementHandlerBase>> RegisteredHandlers;

	/** Current active movement handler */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFVMovementHandlerBase> CurrentMovementHandler;


	/** Time of last transition */
	float LastTransitionTime;

	/** Has the component been initialized? */
	bool bIsInitialized;

public:
	//~=============================================================================
	// Debug
	//~=============================================================================

#if WITH_EDITOR
	/** Validate the configuration */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	AFVCharacter* Character;
	FGameplayTagContainer ActiveTags;
};

