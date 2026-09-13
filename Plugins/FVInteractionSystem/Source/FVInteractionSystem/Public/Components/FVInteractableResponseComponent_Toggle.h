#pragma once

#include "CoreMinimal.h"
#include "FVInteractableComponent.h"
#include "FVInteractableResponseComponent.h"
#include "Core/FVInteractionGameplayTags.h"

#include "FVInteractableResponseComponent_Toggle.generated.h"



UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UFVInteractableResponseComponent_Toggle final : public UFVInteractableResponseComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Toggle")
	bool IsLocked() const { return bIsLocked; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Toggle")
	void SetLocked(bool bLocked);

protected:
	virtual void BindEvents_Implementation(UFVInteractableComponent* Interactable) override;
	virtual void UnbindEvents_Implementation(UFVInteractableComponent* Interactable) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggled(bool bOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnToggleBlocked(bool bOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Toggle")
	void OnLockChanged(bool bLocked);

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle", meta = (Categories = "Interaction.Action"))
	FGameplayTag ToggleActionTag = FVInteractionGameplayTags::Interaction_Action_Open;

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartOpen = false;

	UPROPERTY(EditAnywhere, Category = "Interaction|Toggle")
	bool bStartLocked = false;

private:
	UFUNCTION()
	void OnInteractionEnded(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, const bool bSuccess);

	bool bIsOpen = false;
	bool bIsLocked = false;
};
