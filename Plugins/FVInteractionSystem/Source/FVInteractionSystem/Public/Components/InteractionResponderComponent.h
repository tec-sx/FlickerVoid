#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "InteractionResponderComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;
class UInteractionResponseComponent;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UInteractionResponderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UInteractionResponderComponent();

	UE_API virtual void BeginPlay() override final;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindResponses(UInteractionResponseComponent* Response);

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindInteractableResponses(UInteractableComponent* Interactable);

protected:
	virtual void BindResponses_Implementation(UInteractionResponseComponent* Response) {}
	virtual void BindInteractableResponses_Implementation(UInteractableComponent* Interactable) {}

	UFUNCTION(BlueprintPure, Category = "Interaction|Responder")
	UE_API UInteractionResponseComponent* GetResponse() const { return CachedResponse.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction|Responder")
	UE_API UInteractableComponent* GetInteractable() const { return CachedInteractable.Get(); }

private:
	TWeakObjectPtr<UInteractionResponseComponent> CachedResponse;
	TWeakObjectPtr<UInteractableComponent> CachedInteractable;
};

#undef UE_API
