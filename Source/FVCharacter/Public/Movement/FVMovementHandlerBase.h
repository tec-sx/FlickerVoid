// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVMovementHandlerInfo.h"
#include "FVMovementHandlerBase.generated.h"

class AFVCharacter;
class UFVCharacterMovementComponent;
class UFVMovementHandlerConfigBase;

UCLASS(Abstract, Blueprintable, BlueprintType, meta = (ShowWorldContextPin))
class FLICKERVOIDCHARACTER_API UFVMovementHandlerBase : public UObject
{
	GENERATED_BODY()

public:
	UFVMovementHandlerBase();

	//~=============================================================================
	// Core Lifecycle Events (Implement in Blueprint/Script)
	//~=============================================================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Movement Handler")
	void OnInitialize(AFVCharacter* InCharacter, UFVCharacterMovementComponent* InMovementComponent, const FFVMovementHandlerInfo& InConfig);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement Handler")
	void OnEnter();

	UFUNCTION(BlueprintImplementableEvent, Category = "Movement Handler")
	void GenerateMovement(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "Movement Handler")
	void OnExit();

	UFUNCTION(BlueprintImplementableEvent, Category = "Movement Handler")
	bool Resolve() const;

	void Initialize(AFVCharacter* InCharacter, UFVCharacterMovementComponent* InMovementComponent, const FFVMovementHandlerInfo& InConfig);
	void Enter();
	void TickMovement(float DeltaTime);
	void Exit();

	//~=============================================================================
	// State Access
	//~=============================================================================

	UFUNCTION(BlueprintPure, Category = "Movement Handler")
	AFVCharacter* GetCharacter() const { return Character; }

	UFUNCTION(BlueprintPure, Category = "Movement Handler")
	UFVCharacterMovementComponent* GetMovementComponent() const { return MovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Movement Handler")
	const FFVMovementHandlerInfo& GetConfig() const { return Config; }

	UFUNCTION(BlueprintPure, Category = "Movement Handler")
	bool IsActive() const { return bIsActive; }

	//~=============================================================================
	// Helper Functions
	//~=============================================================================

	UFUNCTION(BlueprintPure, Category = "Movement Handler")
	bool GrantsTag(const FGameplayTag& Tag) const;

private:
	TObjectPtr<AFVCharacter> Character;
	TObjectPtr<UFVCharacterMovementComponent> MovementComponent;
	FFVMovementHandlerInfo Config;
	bool bIsInitialized;
	bool bIsActive;
};
