// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "FVAICharacter.generated.h"

class UFVAIConfigData;
class UFVFlowTriggerComponent;
class UBoxComponent;
class UStateTree;

UCLASS()
class FLICKERVOIDAI_API AFVAICharacter : public ACharacter, public IGameplayTagAssetInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AFVAICharacter();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	
	UFUNCTION(BlueprintCallable, Category = "Tags")
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Tags")
	void AddGameplayTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void RemoveGameplayTag(FGameplayTag Tag);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<UStateTree> StateTree;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<UFVAIConfigData> AIConfig;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer OwnedTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGenericTeamId TeamId = FGenericTeamId(1);
};
