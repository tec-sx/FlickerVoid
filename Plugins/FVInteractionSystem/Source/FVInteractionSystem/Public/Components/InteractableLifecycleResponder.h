#pragma once

#include "Components/InteractionResponderComponent.h"
#include "Core/InteractionTypes.h"
#include "CoreMinimal.h"

#include "InteractableLifecycleResponder.generated.h"

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UInteractableLifecycleResponder final : public UInteractionResponderComponent
{
	GENERATED_BODY()

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void BindInteractableResponses_Implementation(UInteractableComponent* Interactable) override;

private:
	UFUNCTION()
	void HandleInteractionEnded(const FGameplayTag& ActionTag, UInteractorComponent* Interactor, bool bSucceeded);

	void StartCooldown();
	void FinishCooldown();

	FTimerHandle CooldownTimer;
};
