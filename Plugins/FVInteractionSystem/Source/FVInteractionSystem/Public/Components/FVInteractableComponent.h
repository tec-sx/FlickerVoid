

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "GameplayTags.h"
#include "FVInteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UPrimitiveComponent;
class UShapeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractableFocusChanged, bool, bIsInFocus, UFVInteractorComponent*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableStateChanged, EFVInteractableState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCollisionChanged, const TEnumAsByte<ECollisionChannel>&, NewCollisionChannel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionStarted, const FGameplayTag&, ActionTag, UFVInteractorComponent*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInteractionProgress, const FGameplayTag&, ActionTag, UFVInteractorComponent*, Interactor, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInteractionEnded, const FGameplayTag&, ActionTag, UFVInteractorComponent*, Interactor, const bool, bSuccess);

UCLASS(MinimalAPI, ClassGroup=(Interaction), Abstract, Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UFVInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFVInteractableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UE_API FVector GetFocusPoint() const;

	UFUNCTION(BlueprintPure, Category = "Interactable|Detection")
	UE_API float GetDetectionRadius() const { return DetectionRadius; }
	
	UE_API void SetFocused(bool bFocused, UFVInteractorComponent* Interactor);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	UE_API EFVInteractableState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API bool SetState(EFVInteractableState NewState);

	UE_API static bool IsTransitionAllowed(EFVInteractableState From, EFVInteractableState To);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Lifecycle")
	UE_API void ConsumeOffer(const FGameplayTag& ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Dependencies")
	UE_API void ProcessDependencies();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interactable|Dependencies")
	TArray<TObjectPtr<UFVInteractableComponent>> Dependencies;

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API void AddSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API void RemoveSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	UE_API bool CanBeInteractedWith() const { return State == EFVInteractableState::Awake || State == EFVInteractableState::Paused; }
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Events")
	UE_API void StartInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor);
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Events")
	UE_API void ProgressInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, float Progress);
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Events")
	UE_API void EndInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, const bool bSuccess);
	
	UPROPERTY(BlueprintAssignable, Category = "Interactable|State")
	FInteractableFocusChanged FocusChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interactable|State")
	FInteractableStateChanged StateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionStarted InteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionProgress InteractionProgressed;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionEnded InteractionEnded;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag Type;

	const FFVInteractionOffer* FindOffer(const FGameplayTag& InputTag) const;
	const TArray<FFVInteractionOffer>& GetOffers() const { return Offers; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection", meta = (ClampMin = "-1"))
	float DetectionRadius = -1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FName FocusComponentTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Arbitration", meta = (ClampMin = "-1"))
	int32 InteractionWeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Lifecycle", meta = (ClampMin = "0", Units = "s"))
	float CooldownPeriod = -1.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Actions", meta = (ForceInlineRow))
	TArray<FFVInteractionOffer> Offers;

	bool bIsInitialized = false;
	bool bIsInFocus = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	EFVInteractableState State = EFVInteractableState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	FGameplayTagContainer SuppressionReasons;

private:
	void ApplyStateTag(EFVInteractableState OldState, EFVInteractableState NewState) const;
	void StartCooldown();

	TWeakObjectPtr<UPrimitiveComponent> FocusPrimitive;
	FTimerHandle CooldownTimer;
};

#undef UE_API
