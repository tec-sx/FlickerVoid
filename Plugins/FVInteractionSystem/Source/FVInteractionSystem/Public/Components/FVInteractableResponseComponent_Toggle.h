#pragma once

#include "CoreMinimal.h"
#include "FVInteractableComponent.h"
#include "FVInteractableResponseComponent.h"

#include "FVInteractableResponseComponent_Toggle.generated.h"

class UFVLockComponent;

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UFVInteractableResponseComponent_Toggle : public UFVInteractableResponseComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsLocked() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction|Toggle")
	void SetLocked(bool bLocked);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggled(bool bOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggleBlocked(bool bOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnLockChanged(bool bLocked);

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartOpen = false;

	/** Ignored when the owner has a UFVLockComponent; the lock component owns the state then. */
	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartLocked = false;

private:
	virtual void ExecuteAction_Implementation(UFVInteractorComponent* Interactor) override;

	UFUNCTION()
	void HandleLockStateChanged(bool bLocked, AActor* Instigator);

	UPROPERTY(Transient)
	TObjectPtr<UFVLockComponent> LockComponent;

	bool bIsOpen = false;
	bool bIsLocked = false;
};
