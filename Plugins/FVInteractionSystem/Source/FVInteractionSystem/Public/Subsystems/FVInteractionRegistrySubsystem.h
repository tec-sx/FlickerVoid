#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "FVInteractionRegistrySubsystem.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractionDebugSubsystem;
class UFVInteractableComponent;
class UFVInteractorComponent;

/**
 * Tracks interactables and the player interactor. Interactables report entering and leaving the Near band
 * of the Relevance.Interactable group; those in range are awake while an interactor is registered.
 */
UCLASS(MinimalAPI, NotBlueprintable)
class UFVInteractionRegistrySubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UE_API const TArray<TObjectPtr<UFVInteractableComponent>>& GetAllInteractables() const { return RegisteredInteractables; }

	/** Interactables in range while an interactor is registered; empty otherwise. */
	UE_API const TArray<TObjectPtr<UFVInteractableComponent>>& GetActiveInteractables() const;

	void Register(UFVInteractableComponent* Interactable);
	void Unregister(UFVInteractableComponent* Interactable);
	void SetInRange(UFVInteractableComponent* Interactable, bool bInRange);
	void RegisterInteractor(UFVInteractorComponent* InInteractor);
	void UnregisterInteractor();

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UFVInteractorComponent> InteractorPtr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> InRangeInteractables;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> RegisteredInteractables;

#if !UE_BUILD_SHIPPING
	TWeakObjectPtr<UFVInteractionDebugSubsystem> DebugSubsystem;
#endif
};

#undef UE_API
