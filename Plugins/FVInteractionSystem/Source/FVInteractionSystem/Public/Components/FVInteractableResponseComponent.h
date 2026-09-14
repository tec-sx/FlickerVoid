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

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindEvents(UFVInteractableComponent* Interactable);

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void UnbindEvents(UFVInteractableComponent* Interactable);

	UFUNCTION(BlueprintPure, Category = "Interaction|Responder")
	UE_API FGameplayTag GetBoundActionTag() const { return BoundActionTag; }

protected:
	virtual void BindEvents_Implementation(UFVInteractableComponent* Interactable) {}
	virtual void UnbindEvents_Implementation(UFVInteractableComponent* Interactable) {}

	UFUNCTION(BlueprintPure, Category = "Interaction|Responder")
	UE_API bool MatchesBoundAction(const FGameplayTag& ActionTag) const { return ActionTag.MatchesTagExact(BoundActionTag); }

private:
	friend class UFVInteractableComponent;

	void SetBoundActionTag(const FGameplayTag& ActionTag) { BoundActionTag = ActionTag; }

	UPROPERTY(Transient)
	FGameplayTag BoundActionTag;
};

#undef UE_API
