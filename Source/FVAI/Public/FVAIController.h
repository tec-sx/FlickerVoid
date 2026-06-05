// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FVAIController.generated.h"

class AFVCharacter;

/**
 * Base AI Controller for FlickerVoid characters.
 * Handles NPC behavior and takes control when player releases a character.
 */
UCLASS()
class FLICKERVOIDAI_API AFVAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFVAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Get the controlled FVCharacter */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|AI")
	AFVCharacter* GetFVCharacter() const;

	/** Called when regaining control from player */
	UFUNCTION(BlueprintNativeEvent, Category = "FlickerVoid|AI")
	void OnRegainedControlFromPlayer();

protected:
	/** Cached reference to controlled character */
	UPROPERTY()
	TObjectPtr<AFVCharacter> CachedCharacter;
};