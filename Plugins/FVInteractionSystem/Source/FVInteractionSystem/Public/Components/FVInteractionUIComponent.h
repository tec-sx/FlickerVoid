#pragma once

#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

#include "FVInteractionUIComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractorComponent;
class UFVInteractableComponent;
class UFVInteractionUISettings;
class UUserWidget;
class UFVInteractionWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOffersChanged, const TArray<FFVInteractionOfferData>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOfferProgress, const FGameplayTag, ActionTag, float, Progress);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent, RequiresInteractor))
class UFVInteractionUIComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionUIComponent();

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API const TArray<FFVInteractionOfferData>& GetOffers() const { return Offers; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API UUserWidget* GetWidget() const { return Widget; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API const FSlateBrush& GetCurrentFocusIndicatorBrush() const { return CurrentFocusIndicatorBrush; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Prompt")
	UE_API bool GetFocusWidgetPosition(FVector2D& OutPosition) const;

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
	void CacheFocusIndicators();
	const FSlateBrush& ResolveFocusIndicator(const FGameplayTag& InteractableType) const;
	void UpdateFocusIndicator();
	void UpdateOverlay();
	FVector GetFocusWorldLocation() const;
	void PushOffersToWidget();
	UFVInteractionWidget* GetInteractionWidget() const;

	UFUNCTION()
	void OnFocusLost(UFVInteractableComponent* OldTarget);

	UFUNCTION()
	void OnInteractionEnded(const FFVInteractionCommit& Commit, bool bSuccess);

	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void OnFocusChanged(UFVInteractableComponent* NewTarget);

	UFUNCTION()
	void OnModeChanged(FGameplayTag NewMode, FGameplayTag OldMode);

	UFUNCTION()
	void OnOffersChanged(const TArray<FFVInteractionOfferData>& InOffers);

	UFUNCTION()
	void OnInteractionProgress(const FFVInteractionCommit& Commit, float Progress);

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractorComponent> Interactor;

	UPROPERTY(Transient)
	TArray<FFVInteractionOfferData> Offers;

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractableComponent> FocusedTarget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> Widget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OverlayWidget;

	UPROPERTY(Transient)
	TSubclassOf<UUserWidget> OverlayClass;

	UPROPERTY(Transient)
	FSlateBrush DefaultFocusIndicatorBrush;

	UPROPERTY(Transient)
	TMap<FName, FSlateBrush> FocusIndicatorOverrides;

	UPROPERTY(Transient)
	FSlateBrush CurrentFocusIndicatorBrush;

	FGameplayTag CurrentInteractableType;
};

#undef UE_API
