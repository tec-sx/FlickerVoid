#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "InteractionResponseComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractionSignalComponent;
class UInteractorComponent;
class UInteractableComponent;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UInteractionResponseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UInteractionResponseComponent();

	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void BindSignals(UInteractionSignalComponent* Signal);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Responder")
	void UnbindSignals(UInteractionSignalComponent* Signal);
	
protected:
	virtual void BindSignals_Implementation(UInteractionSignalComponent* Signal) {}
	virtual void UnbindSignals_Implementation(UInteractionSignalComponent* Signal) {}

	UFUNCTION(BlueprintPure, Category = "Interaction|Signal")
	UE_API TArray<UInteractionSignalComponent*> GetSignals() const;

private:
	TArray<TWeakObjectPtr<UInteractionSignalComponent>> Signals;
};

#undef UE_API
