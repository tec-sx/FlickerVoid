#pragma once

#include "Components/InteractionResponseComponent.h"
#include "CoreMinimal.h"
#include "InteractableComponent.h"

#include "Response_Toggle.generated.h"

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UResponse_Toggle final : public UInteractionResponseComponent
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
	virtual void BindSignals_Implementation(UInteractionSignalComponent* Signal) override;
	virtual void UnbindSignals_Implementation(UInteractionSignalComponent* Signal) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggled(bool bOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggleBlocked(bool bOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnLockChanged(bool bLocked);

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle", meta = (Categories = "Interaction.Action"))
	FGameplayTag ToggleActionTag;

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bInitialState = false;

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartsLocked = false;

private:
	UFUNCTION()
	void OnInteractionRequested(const FInteractionCommit& Commit);

	bool bIsOn = false;
	bool bIsLocked = false;
};
