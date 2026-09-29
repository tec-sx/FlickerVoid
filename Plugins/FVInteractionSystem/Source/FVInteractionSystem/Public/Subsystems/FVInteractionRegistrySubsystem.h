#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "FVInteractionRegistrySubsystem.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractionDebugSubsystem;
class UFVInteractableComponent;
class UFVInteractorComponent;

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

	UE_API const TArray<TObjectPtr<UFVInteractableComponent>>& GetAllInteractables() const { return RegisteredInteractables; }
	UE_API const TArray<TObjectPtr<UFVInteractableComponent>>& GetActiveInteractables() const { return ActiveInteractables; }
	
	void Register(UFVInteractableComponent* Interactable);
	void Unregister(UFVInteractableComponent* Interactable);
	void RegisterInteractor(UFVInteractorComponent* InInteractor);
	void UnregisterInteractor();
	
	
private:
	void Update();

	UPROPERTY(Transient)
	TWeakObjectPtr<UFVInteractorComponent> InteractorPtr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> ActiveInteractables;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> RegisteredInteractables;
	
	float TickInterval = 0.01f;
	float TimeSinceLastTick = 0.f;
	
#if !UE_BUILD_SHIPPING
	TWeakObjectPtr<UFVInteractionDebugSubsystem> DebugSubsystem;
#endif
};

#undef UE_API