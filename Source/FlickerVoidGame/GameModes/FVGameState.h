// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameStateBase.h"
#include "AbilitySystemInterface.h"

#include "FVGameState.generated.h"


class UAbilitySystemComponent;
class UFVAbilitySystemComponent;

UCLASS(Config = Game)
class FLICKERVOID_API AFVGameState : public AGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFVGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

private:
	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "FV|GameState")
	TObjectPtr<UFVAbilitySystemComponent> AbilitySystemComponent;
};

#undef UE_API