#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "InteractionResponseComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractorComponent;
class UInteractableComponent;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UInteractionResponseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UInteractionResponseComponent();

	UE_API virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindInteractions(UInteractorComponent* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void UnbindInteractions(UInteractorComponent* Interactor);
	
protected:
	virtual void BindInteractions_Implementation(UInteractorComponent* Interactor) {}
	virtual void UnbindInteractions_Implementation(UInteractorComponent* Interactor) {}

	UFUNCTION(BlueprintPure, Category = "Interaction|Responder")
	UE_API UInteractorComponent* GetInteractor() const { return Interactor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction|Responder")
	UE_API UInteractableComponent* GetInteractable() const { return Interactable.Get(); }

private:
	TWeakObjectPtr<UInteractorComponent> Interactor;
	TWeakObjectPtr<UInteractableComponent> Interactable;
};

#undef UE_API
