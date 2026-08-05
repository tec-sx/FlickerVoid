// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "FVAICharacter.generated.h"

class UFVFlowTriggerComponent;
class UBoxComponent;
class UStateTree;

UCLASS()
class FLICKERVOIDAI_API AFVAICharacter : public ACharacter, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AFVAICharacter();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	
	UFUNCTION(BlueprintCallable, Category = "Tags")
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Tags")
	void AddGameplayTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void RemoveGameplayTag(FGameplayTag Tag);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<UStateTree> StateTree;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character)
	FGameplayTag FlowIdentity;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer OwnedTags;
};
