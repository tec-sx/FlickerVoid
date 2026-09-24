#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "FVInteractionRegistrySubsystem.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractionDebugSubsystem;
class UFVInteractableComponent;
class UFVInteractorComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInRangeSetChanged, bool, bHasAnyInRange);

UCLASS(MinimalAPI, NotBlueprintable)
class UFVInteractionRegistrySubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UE_API void Register(UFVInteractableComponent* Interactable);
	UE_API void Unregister(UFVInteractableComponent* Interactable);
	UE_API void RegisterInteractor(UFVInteractorComponent* InInteractor);
	UE_API void UnregisterInteractor();
	
	const TArray<TObjectPtr<UFVInteractableComponent>>& GetAllInteractables() const { return RegisteredInteractables; }
	const TArray<TObjectPtr<UFVInteractableComponent>>& GetActiveInteractables() const { return ActiveInteractables; }
	
private:
	void Update();
	bool IsActive(const UFVInteractableComponent* Interactable);

	UPROPERTY(Transient)
	TWeakObjectPtr<UFVInteractorComponent> InteractorPtr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> ActiveInteractables;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> RegisteredInteractables;
	
	float TickInterval;
	float TimeSinceLastTick;
	
#if !UE_BUILD_SHIPPING
	TWeakObjectPtr<UFVInteractionDebugSubsystem> DebugSubsystem;
#endif
};

#undef UE_API