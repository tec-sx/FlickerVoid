#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "FVInteractableResponseComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractableComponent;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractableResponseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractableResponseComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Response")
	void ExecuteAction(UFVInteractorComponent* Interactor);

	UFUNCTION(BlueprintPure, Category = "Interaction|Response")
	UE_API FGameplayTag GetActionTag() const { return ActionTag; }

protected:
	virtual void ExecuteAction_Implementation(UFVInteractorComponent* Interactor) {}

private:
	friend class UFVInteractableComponent;

	void SetActionTag(const FGameplayTag& InActionTag) { ActionTag = InActionTag; }

	UPROPERTY(Transient)
	FGameplayTag ActionTag;
};

#undef UE_API
