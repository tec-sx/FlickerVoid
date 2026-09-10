

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"
#include "GameplayTags.h"
#include "InteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractorComponent;
class UPrimitiveComponent;
class UShapeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableStateChanged, EInteractableState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCollisionChanged, const TEnumAsByte<ECollisionChannel>&, NewCollisionChannel);

UCLASS(MinimalAPI, ClassGroup=(Interaction), Abstract, Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UE_API FVector GetFocusPoint() const;

	UFUNCTION(BlueprintPure, Category = "Interactable|Detection")
	UE_API float GetDetectionRadius() const { return DetectionRadius; }
	
	UE_API void SetFocused(bool bFocused, UInteractorComponent* Interactor);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	UE_API EInteractableState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API bool SetState(EInteractableState NewState);

	UE_API static bool IsTransitionAllowed(EInteractableState From, EInteractableState To);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Lifecycle")
	UE_API void ConsumeOffer(const FGameplayTag& ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Dependencies")
	UE_API void ProcessDependencies();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interactable|Dependencies")
	TArray<TObjectPtr<UInteractableComponent>> Dependencies;

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API void AddSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API void RemoveSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	UE_API bool CanBeInteractedWith() const { return State == EInteractableState::Awake || State == EInteractableState::Paused; }

	UPROPERTY(BlueprintAssignable, Category = "Interactable|State")
	FInteractableStateChanged StateChanged;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag Type;

	const FInteractionOffer* FindOffer(const FGameplayTag& InputTag) const;
	const TArray<FInteractionOffer>& GetOffers() const { return Offers; }

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
	TArray<FInteractionOffer> Offers;

	bool bIsInitialized = false;
	bool bIsInFocus = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	EInteractableState State = EInteractableState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	FGameplayTagContainer SuppressionReasons;

private:
	void ApplyStateTag(EInteractableState OldState, EInteractableState NewState) const;
	void StartCooldown();

	TWeakObjectPtr<UPrimitiveComponent> FocusPrimitive;
	FTimerHandle CooldownTimer;
};

#undef UE_API
