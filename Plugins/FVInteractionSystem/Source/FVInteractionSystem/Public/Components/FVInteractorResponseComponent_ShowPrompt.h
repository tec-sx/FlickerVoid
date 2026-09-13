#pragma once

#include "Components/FVInteractorResponseComponent.h"
#include "Core/FVInteractionTypes.h"
#include "CoreMinimal.h"

#include "FVInteractorResponseComponent_ShowPrompt.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPromptsChanged, const TArray<FFVInteractionOffer>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPromptProgress, const FGameplayTag&, ActionTag, float, Progress);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractorResponseComponent_ShowPrompt final : public UFVInteractorResponseComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API const TArray<FFVInteractionOffer>& GetPrompts() const { return Prompts; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Prompt")
	FOnPromptsChanged OnPromptsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Prompt")
	FOnPromptProgress OnPromptProgress;

protected:
	virtual void BindEvents_Implementation(UFVInteractorComponent* Interactor) override;
	virtual void UnbindEvents_Implementation(UFVInteractorComponent* Interactor) override;

private:
	UFUNCTION()
	void OnFocusChanged(UFVInteractableComponent* NewTarget);

	UFUNCTION()
	void OnOffersChanged(const TArray<FFVInteractionOffer>& Offers);

	UFUNCTION()
	void OnInteractionProgress(const FFVInteractionCommit& Commit, float Progress);

	UPROPERTY(Transient)
	TArray<FFVInteractionOffer> Prompts;

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractableComponent> FocusedTarget;
};

#undef UE_API
