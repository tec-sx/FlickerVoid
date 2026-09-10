#pragma once

#include "Components/InteractionResponderComponent.h"
#include "Core/InteractionTypes.h"
#include "CoreMinimal.h"

#include "InteractableStateVFXResponder.generated.h"

class UFXSystemAsset;
class USoundBase;

USTRUCT(BlueprintType)
struct FInteractableStateEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EInteractableState State = EInteractableState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UFXSystemAsset> Effect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound = nullptr;
};

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UInteractableStateVFXResponder final : public UInteractionResponderComponent
{
	GENERATED_BODY()

protected:
	virtual void BindInteractableResponses_Implementation(UInteractableComponent* Interactable) override;

	UPROPERTY(EditAnywhere, Category = "Interaction|Feedback", meta = (TitleProperty = "State"))
	TArray<FInteractableStateEffect> StateEffects;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Feedback")
	void OnStateEffect(EInteractableState NewState, UFXSystemAsset* Effect);

private:
	UFUNCTION()
	void HandleStateChanged(EInteractableState NewState);
};
