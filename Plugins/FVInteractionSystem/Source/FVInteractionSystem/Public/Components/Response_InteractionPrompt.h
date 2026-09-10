#pragma once

#include "Components/InteractionResponseComponent.h"
#include "Core/InteractionTypes.h"
#include "CoreMinimal.h"

#include "Response_InteractionPrompt.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPromptsChanged, const TArray<FInteractionOffer>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPromptProgress, const FGameplayTag&, ActionTag, float, Progress);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UResponse_InteractionPrompt final : public UInteractionResponseComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API const TArray<FInteractionOffer>& GetPrompts() const { return Prompts; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Prompt")
	FOnPromptsChanged OnPromptsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Prompt")
	FOnPromptProgress OnPromptProgress;

protected:
	virtual void BindSignals_Implementation(UInteractionSignalComponent* Signal) override;
	virtual void UnbindSignals_Implementation(UInteractionSignalComponent* Signal) override;

private:
	UFUNCTION()
	void OnFocusChanged(UInteractableComponent* NewTarget);

	UFUNCTION()
	void OnOffersChanged(const TArray<FInteractionOffer>& Offers);

	UFUNCTION()
	void OnInteractionProgress(const FInteractionCommit& Commit, float Progress);

	UPROPERTY(Transient)
	TArray<FInteractionOffer> Prompts;

	UPROPERTY(Transient)
	TObjectPtr<UInteractableComponent> FocusedTarget;
};

#undef UE_API
