// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/FVCharacter.h"
#include "UObject/Object.h"
#include "FVPlayerCharacter.generated.h"

class UFVInteractableComponent;
class UFVAbilitySystemComponent;
class UFVInteractorResponseComponent_ActivateAbility;
class UFVInteractorResponseComponent;
class UFVInteractorComponent;

UCLASS()
class FLICKERVOID_API AFVPlayerCharacter : public AFVCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFVPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|PlayerState")
	UFVAbilitySystemComponent* GetFVAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Interaction")
	UFVInteractorComponent* GetInteractor() const { return Interactor; }
	
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Interaction")
	UFVInteractableComponent* GetFocusedInteractable() const;
	
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Interaction")
	AActor* GetFocusedActor() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|Interaction")
	TObjectPtr<UFVInteractorComponent> Interactor;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|Interaction")
	TObjectPtr<UFVInteractorResponseComponent_ActivateAbility> ActivateAbilityResponse;
};
