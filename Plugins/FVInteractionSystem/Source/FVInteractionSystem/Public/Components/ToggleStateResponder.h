#pragma once

#include "Components/InteractionResponderComponent.h"
#include "Core/InteractionTypes.h"
#include "CoreMinimal.h"

#include "ToggleStateResponder.generated.h"

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UToggleStateResponder final : public UInteractionResponderComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsOn() const { return bIsOn; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsLocked() const { return bIsLocked; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Toggle")
	void SetLocked(bool bLocked);

protected:
	virtual void BindInteractableResponses_Implementation(UInteractableComponent* Interactable) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggled(bool bOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggleBlocked(bool bOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnLockChanged(bool bLocked);

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle", meta = (Categories = "Interaction.Action"))
	FGameplayTag ToggleActionTag;

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartsOn = false;

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartsLocked = false;

private:
	UFUNCTION()
	void HandleInteractionEnded(const FGameplayTag& ActionTag, UInteractorComponent* Interactor, bool bSucceeded);

	bool bIsOn = false;
	bool bIsLocked = false;
};
