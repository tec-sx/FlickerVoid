// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/FVCharacter.h"
#include "UObject/Object.h"
#include "FVPlayerCharacter.generated.h"

class UFVInteractionOfferComponent;
class UFVInteractionInstigatorComponent;
class UFVAbilitySystemComponent;

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
	UFVInteractionInstigatorComponent* GetInteraction() const;
	
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Interaction")
	UFVInteractionOfferComponent* GetOffers() const;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|Interaction")
	TObjectPtr<UFVInteractionInstigatorComponent> InteractionInstigator;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|Interaction")
	TObjectPtr<UFVInteractionOfferComponent> InteractionOffers;
	
	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVAbilitySystemComponent> AbilitySystemComponent;
};
