#pragma once

#include "CoreMinimal.h"
#include "FVInteractableComponent.h"
#include "FVInteractableResponseComponent.h"

#include "FVInteractableResponseComponent_Toggle.generated.h"

class UFVInteractorComponent;
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

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggled(bool bOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle", meta = (ToolTip = "The lock blocked the interactor's toggle. If this unlocks the lock, e.g. with a key the interactor carries, the toggle goes ahead."))
	void OnToggleBlocked(bool bOpen, UFVInteractorComponent* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnLockChanged(bool bLocked);

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartOpen = false;

private:
	virtual void ExecuteAction_Implementation(UFVInteractorComponent* Interactor) override;

	UFUNCTION()
	void HandleLockStateChanged(bool bLocked, AActor* Instigator);

	UPROPERTY(Transient)
	TObjectPtr<UFVLockComponent> LockComponent;

	bool bIsOpen = false;
};
