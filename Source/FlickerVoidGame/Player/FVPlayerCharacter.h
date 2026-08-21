// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/FVCharacter.h"
#include "UObject/Object.h"
#include "FVPlayerCharacter.generated.h"

class UFVAbilitySystemComponent;
class UInteractorComponent;

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
	UInteractorComponent* GetInteractor() const { return Interactor; }

	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|Interaction")
	TObjectPtr<UInteractorComponent> Interactor;
};
