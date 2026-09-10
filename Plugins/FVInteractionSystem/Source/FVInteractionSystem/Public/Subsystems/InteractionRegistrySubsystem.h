#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "InteractionRegistrySubsystem.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;
class UInteractorComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInRangeSetChanged, bool, bHasAnyInRange);

USTRUCT()
struct FInteractorRangeSet
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UInteractorComponent> Interactor;

	UPROPERTY()
	TArray<TObjectPtr<UInteractableComponent>> InRange;
};

UCLASS(MinimalAPI, NotBlueprintable)
class UInteractionRegistrySubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	UE_API void Register(UInteractableComponent* Interactable);
	UE_API void Unregister(UInteractableComponent* Interactable);
	UE_API void RegisterInteractor(UInteractorComponent* Interactor);
	UE_API void UnregisterInteractor(UInteractorComponent* Interactor);

	UE_API void QueryInRange(const FVector& Origin, float MaxRadius, TArray<UInteractableComponent*>& OutResults) const;
	UE_API const TArray<TObjectPtr<UInteractableComponent>>& GetInRangeSet(const UInteractorComponent* Interactor) const;

	const TArray<TObjectPtr<UInteractableComponent>>& GetAll() const { return Interactables; }

	UPROPERTY(BlueprintAssignable)
	FOnInRangeSetChanged OnInRangeSetChanged;

private:
	void RunBroadPhase();
	void StartBroadPhase();
	bool IsInRange(const UInteractableComponent* Interactable) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInteractableComponent>> Interactables;

	UPROPERTY(Transient)
	TArray<FInteractorRangeSet> InteractorRanges;

	FTimerHandle BroadPhaseTimer;
};

#undef UE_API