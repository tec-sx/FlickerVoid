#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/InteractionTypes.h"
#include "UI/InteractionPromptStyle.h"

#include "InteractionPromptWidget.generated.h"

class UInteractorComponent;
class UInteractionPromptStyleAsset;
class UInputAction;

/**
 * Receives resolved prompts from an interactor and maps them to presentation data.
 * Holds no gameplay query logic; it is fed by UInteractorComponent::OnOffersChanged.
 */
UCLASS(Abstract)
class FVINTERACTIONSYSTEM_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	void BindToInteractor(UInteractorComponent* InInteractor);

	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	void UnbindFromInteractor();

	virtual void NativeDestruct() override;

protected:
	/** Rebuild the prompt list. Use ResolveStyle and ResolveKeyForInputTag for each entry. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|UI")
	void OnPromptsUpdated(const TArray<FInteractionPrompt>& Prompts);

	UFUNCTION(BlueprintPure, Category = "Interaction|UI")
	bool ResolveStyle(FGameplayTag ActionTag, FInteractionPromptStyle& OutStyle) const;

	/** Resolves the key currently bound to an input tag. Override to use the game's own input config. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|UI")
	bool ResolveKeyForInputTag(FGameplayTag InputTag, FKey& OutKey) const;
	virtual bool ResolveKeyForInputTag_Implementation(FGameplayTag InputTag, FKey& OutKey) const;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	TObjectPtr<UInteractionPromptStyleAsset> StyleAsset;

	/** Fallback tag to input action mapping used by the default key resolution. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI", meta = (ForceInlineRow, Categories = "Input.Action"))
	TMap<FGameplayTag, TObjectPtr<const UInputAction>> InputActions;

private:
	UFUNCTION()
	void HandleOffersChanged(const TArray<FInteractionPrompt>& Prompts);

	TWeakObjectPtr<UInteractorComponent> Interactor;
};
