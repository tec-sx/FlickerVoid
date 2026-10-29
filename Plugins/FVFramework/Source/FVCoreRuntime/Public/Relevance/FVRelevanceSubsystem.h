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

	/** Registered components of Group, closest first. */
	void GetComponents(const FGameplayTag& Group, TArray<UFVRelevanceComponent*>& OutComponents) const;

	/** Components of Group in its Near band, closest first. Stops at the first one too far to be Near. */
	void GetNearComponents(const FGameplayTag& Group, TArray<UFVRelevanceComponent*>& OutComponents) const;

	bool HasNearComponents(const FGameplayTag& Group) const;

	/** Viewpoints used by the last update. */
	const TArray<FTransform>& GetViewpoints() const { return Viewpoints; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	USignificanceManager* GetSignificanceManager() const;
	/** Visits Near components of Group closest first until Visit returns false. */
	void ForEachNearComponent(const FGameplayTag& Group, TFunctionRef<bool(UFVRelevanceComponent*)> Visit) const;
	void GatherViewpoints();

	TArray<FTransform> Viewpoints;
	TArray<TWeakObjectPtr<UFVRelevanceComponent>> ChangedComponents;
	double LastUpdateTime = 0.0;
};
