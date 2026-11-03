// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FVAICharacter.generated.h"

class UFVAIConfigData;
class UFVFlowTriggerComponent;
class UBoxComponent;
class UStateTree;

UCLASS()
class FLICKERVOIDAI_API AFVAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFVAICharacter();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<UStateTree> StateTree;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<UFVAIConfigData> AIConfig;
};
