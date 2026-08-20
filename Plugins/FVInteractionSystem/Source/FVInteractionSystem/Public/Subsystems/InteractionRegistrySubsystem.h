#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "InteractionRegistrySubsystem.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;

UCLASS(MinimalAPI)
class UInteractionRegistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	UE_API void Register(UInteractableComponent* Interactable);
	UE_API void Unregister(UInteractableComponent* Interactable);
	UE_API void QueryInRange(const FVector& Origin, float MaxRadius, TArray<UInteractableComponent*>& OutResults) const;

	const TArray<TObjectPtr<UInteractableComponent>>& GetAll() const { return Interactables; }

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInteractableComponent>> Interactables;
};

#undef UE_API