#pragma once

#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "CoreMinimal.h"

#include "FVInteractionUIComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractorComponent;
class UFVInteractableComponent;
class UFVInteractionUISettings;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOffersChanged, TArray<FFVInteractionOffer>, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOfferProgress, const FGameplayTag, ActionTag, float, Progress);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent, RequiresInteractor))
class UFVInteractionUIComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionUIComponent();

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API const TArray<FFVInteractionOffer>& GetOffers() const { return Offers; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API UUserWidget* GetWidget() const { return Widget; }

	UE_API const UFVInteractionUISettings* GetUISettings() const;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Prompt")
	FOffersChanged OffersChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Prompt")
	FOfferProgress OfferProgress;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|UI")
	TObjectPtr<UFVInteractionUISettings> UISettingsOverride;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ShowWidget();
	void HideWidget();

	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void OnFocusChanged(UFVInteractableComponent* NewTarget);

	UFUNCTION()
	void OnOffersChanged(const TArray<FFVInteractionOffer>& InOffers);

	UFUNCTION()
	void OnInteractionProgress(const FFVInteractionCommit& Commit, float Progress);

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractorComponent> Interactor;

	UPROPERTY(Transient)
	TArray<FFVInteractionOffer> Offers;

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractableComponent> FocusedTarget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> Widget;
};

#undef UE_API
