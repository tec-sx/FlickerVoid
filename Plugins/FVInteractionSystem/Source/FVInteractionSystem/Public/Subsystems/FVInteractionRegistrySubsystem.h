#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "FVInteractionRegistrySubsystem.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractableComponent;
class UFVInteractorComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInRangeSetChanged, bool, bHasAnyInRange);

USTRUCT()
struct FInteractorRangeSet
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UFVInteractorComponent> Interactor;

	UPROPERTY()
	TArray<TObjectPtr<UFVInteractableComponent>> InRange;
};

UCLASS(MinimalAPI, NotBlueprintable)
class UFVInteractionRegistrySubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	UE_API void Register(UFVInteractableComponent* Interactable);
	UE_API void Unregister(UFVInteractableComponent* Interactable);
	UE_API void RegisterInteractor(UFVInteractorComponent* Interactor);
	UE_API void UnregisterInteractor(UFVInteractorComponent* Interactor);

	UE_API void QueryInRange(const FVector& Origin, float MaxRadius, TArray<UFVInteractableComponent*>& OutResults) const;
	UE_API const TArray<TObjectPtr<UFVInteractableComponent>>& GetInRangeSet(const UFVInteractorComponent* Interactor) const;

	const TArray<TObjectPtr<UFVInteractableComponent>>& GetAll() const { return Interactables; }

	UPROPERTY(BlueprintAssignable)
	FOnInRangeSetChanged OnInRangeSetChanged;

private:
	void RunBroadPhase();
	void StartBroadPhase();
	bool IsInRange(const UFVInteractableComponent* Interactable) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVInteractableComponent>> Interactables;

	UPROPERTY(Transient)
	TArray<FInteractorRangeSet> InteractorRanges;

	FTimerHandle BroadPhaseTimer;
};

#undef UE_API