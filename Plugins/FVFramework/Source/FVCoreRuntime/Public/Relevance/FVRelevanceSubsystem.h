#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVRelevanceSubsystem.generated.h"

class USignificanceManager;
class UFVRelevanceComponent;

/**
 * Drives the engine's Significance Manager from the local players' viewpoints. Relevance components
 * register here; the manager scores them in parallel and their band changes are applied afterwards
 * on the game thread.
 */
UCLASS()
class FVCORERUNTIME_API UFVRelevanceSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVRelevanceSubsystem* Get(const UObject* WorldContext);

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	bool Register(UFVRelevanceComponent* Component);
	void Unregister(UFVRelevanceComponent* Component);

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (GameplayTagFilter = "Relevance", ToolTip = "Registered components of the group, closest first."))
	void GetComponents(const FGameplayTag& Group, TArray<UFVRelevanceComponent*>& OutComponents) const;

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (GameplayTagFilter = "Relevance", ToolTip = "Components of the group in its Near band, closest first."))
	void GetNearComponents(const FGameplayTag& Group, TArray<UFVRelevanceComponent*>& OutComponents) const;

	UFUNCTION(BlueprintPure, Category = "FV|Relevance", meta = (GameplayTagFilter = "Relevance", ToolTip = "True when any component of the group is in its Near band."))
	bool HasNearComponents(const FGameplayTag& Group) const;

	const TArray<FTransform>& GetViewpoints() const { return Viewpoints; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	USignificanceManager* GetSignificanceManager() const;
	void ForEachNearComponent(const FGameplayTag& Group, TFunctionRef<bool(UFVRelevanceComponent*)> Visit) const;
	void GatherViewpoints();

	TArray<FTransform> Viewpoints;
	TArray<TWeakObjectPtr<UFVRelevanceComponent>> ChangedComponents;
	double LastUpdateTime = 0.0;
};
