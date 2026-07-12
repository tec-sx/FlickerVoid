// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FVAICharacter.generated.h"

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
	
	UFUNCTION(BlueprintPure, Category = "AICharacter", meta = (ReturnDisplayName = "State Tree"))
	UStateTree* GetStateTree() const { return StateTree; }
	
	UFUNCTION(BlueprintPure, Category = "AICharacter", meta = (ReturnDisplayName = "Interaction Sphere"))
	UBoxComponent* GetInteractionZone() const { return InteractionZone; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AICharacter", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionZone;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AICharacter", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTree> StateTree;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AICharacter", meta = (AllowPrivateAccess = "true"))
	float InteractionRadius = 100.f;
};
