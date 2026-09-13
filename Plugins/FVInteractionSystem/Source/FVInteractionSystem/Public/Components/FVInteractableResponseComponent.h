#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "FVInteractableResponseComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractableComponent;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractableResponseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UFVInteractableResponseComponent();

	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindEvents(UFVInteractableComponent* Signal);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void UnbindEvents(UFVInteractableComponent* Signal);
	
protected:
	virtual void BindEvents_Implementation(UFVInteractableComponent* Signal) {}
	virtual void UnbindEvents_Implementation(UFVInteractableComponent* Signal) {}
};

#undef UE_API
