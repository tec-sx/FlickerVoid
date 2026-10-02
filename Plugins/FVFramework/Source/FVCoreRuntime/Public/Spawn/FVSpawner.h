#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Conditions/FVCondition.h"
#include "Data/FVDefinition.h"
#include "FVSpawner.generated.h"

UCLASS(BlueprintType)
class FVCORERUNTIME_API UFVSpawnDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UDataAsset> Payload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	FFVConditionSet SpawnWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	FFVConditionSet DespawnWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	bool bRespawn = false;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnSpawnerChanged);

UCLASS(Blueprintable)
class FVCORERUNTIME_API AFVSpawner : public AActor
{
	GENERATED_BODY()

public:
	AFVSpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void Evaluate();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void Despawn();

	UFUNCTION(BlueprintPure, Category = "Spawn")
	AActor* GetSpawnedActor() const { return SpawnedActor; }

	UFUNCTION(BlueprintPure, Category = "Spawn")
	UFVSpawnDefinition* GetDefinition() const { return Definition; }

	UPROPERTY(BlueprintAssignable, Category = "Spawn")
	FFVOnSpawnerChanged OnSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Spawn")
	FFVOnSpawnerChanged OnDespawned;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UFVSpawnDefinition> Definition;

private:
	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);
	void Spawn();
	FFVConditionContext MakeContext() const;

	UFUNCTION()
	void HandleSpawnedDestroyed(AActor* Actor);

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedActor;

	FDelegateHandle FactHandle;
	bool bHasSpawned = false;
};