#pragma once

#include "Components/InteractionResponseComponent.h"
#include "CoreMinimal.h"
#include "InteractableComponent.h"

#include "InteractableToggleComponent.generated.h"

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UInteractableToggleComponent final : public UInteractionResponseComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void BindInteractions_Implementation(UInteractorComponent* Interactor) override;
	virtual void UnbindInteractions_Implementation(UInteractorComponent* Interactor) override;
	
	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsOn() const { return bIsOn; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsLocked() const { return bIsLocked; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Toggle")
	void SetLocked(bool bLocked);

protected:
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
	void HandleInteractionEnded(const FInteractionCommit& Commit, bool bSuccess);

	bool bIsOn = false;
	bool bIsLocked = false;
};
