#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/InteractionTypes.h"
#include "UI/InteractionPromptStyle.h"

#include "InteractionPromptWidget.generated.h"

class UInteractorComponent;
class UInteractionPromptStyleAsset;

UCLASS(Abstract)
class FVINTERACTIONSYSTEMUI_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	void BindToInteractor(UInteractorComponent* InInteractor);

	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	void UnbindFromInteractor();

	virtual void NativeDestruct() override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|UI")
	void OnPromptsUpdated(const TArray<FInteractionPrompt>& Prompts);

	UFUNCTION(BlueprintPure, Category = "Interaction|UI")
	bool ResolveStyle(FGameplayTag ActionTag, FInteractionPromptStyle& OutStyle) const;

	UFUNCTION(BlueprintPure, Category = "Interaction|UI")
	bool ResolveKeyBinding(FGameplayTag InputTag, FInteractionKeyBinding& OutBinding) const;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	TObjectPtr<UInteractionPromptStyleAsset> StyleAsset;

private:
	UFUNCTION()
	void HandleOffersChanged(const TArray<FInteractionPrompt>& Prompts);

	TWeakObjectPtr<UInteractorComponent> Interactor;
};
