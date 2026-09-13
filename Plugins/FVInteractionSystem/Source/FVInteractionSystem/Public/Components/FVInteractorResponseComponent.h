#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "FVInteractorResponseComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractorComponent;
class UFVInteractableComponent;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractorResponseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UFVInteractorResponseComponent();

	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindEvents(UFVInteractorComponent* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void UnbindEvents(UFVInteractorComponent* Interactor);
	
protected:
	virtual void BindEvents_Implementation(UFVInteractorComponent* Interactor) {}
	virtual void UnbindEvents_Implementation(UFVInteractorComponent* Interactor) {}
};

#undef UE_API
