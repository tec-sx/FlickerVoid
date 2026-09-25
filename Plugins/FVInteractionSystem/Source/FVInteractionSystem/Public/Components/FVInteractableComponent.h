

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "GameplayTags.h"
#include "FVInteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UPrimitiveComponent;
class UShapeComponent;
class UFVInteractableResponseComponent;
class UFVInteractionRegistrySubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableFoundInteractor, UFVInteractorComponent*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableLostInteractor, UFVInteractorComponent*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableStateChanged, EFVInteractableState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionStarted, const FGameplayTag&, ActionTag, UFVInteractorComponent*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInteractionProgress, const FGameplayTag&, ActionTag, UFVInteractorComponent*, Interactor, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInteractionEnded, const FGameplayTag&, ActionTag, UFVInteractorComponent*, Interactor, const bool, bSuccess);

UCLASS(MinimalAPI, ClassGroup=(Interaction), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UFVInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFVInteractableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UE_API FVector GetFocusPoint() const;

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	UE_API EFVInteractableState GetState() const { return State; }

	UE_API static bool IsTransitionAllowed(EFVInteractableState From, EFVInteractableState To);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Lifecycle")
	void ActivateInteractions();
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Lifecycle")
	void DeactivateInteractions();
    	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Lifecycle")
	UE_API bool IsInteractionActive() const { return State != EFVInteractableState::Idle; }
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Lifecycle")
	UE_API void ConsumeOffer(const FGameplayTag& InputTag);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Responses")
	UE_API void BindResponse(FGameplayTag ActionTag, UFVInteractableResponseComponent* Response);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Responses")
	UE_API void UnbindResponse(UFVInteractableResponseComponent* Response);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Dependencies")
	UE_API void ProcessDependencies();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interactable|Dependencies")
	TArray<TObjectPtr<UFVInteractableComponent>> Dependencies;

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API void AddSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	UE_API void RemoveSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	UE_API bool CanInteract() const;

#pragma region Detection
	
	UFUNCTION(BlueprintPure, Category = "Interactable|Detection")
	ECollisionChannel GetCollisionChannel() const { return CollisionChannel; }
	
	UFUNCTION(BlueprintPure, Category = "Interactable|Detection")
	FGameplayTagContainer GetCompatibleInteractorTags() const { return CompatibleInteractorTags; }
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Detection")
	void AddCompatibleInteractorTag(const FGameplayTag Tag) { CompatibleInteractorTags.AddTag(Tag); }
	
	UFUNCTION(BlueprintPure, Category = "Interactable|Detection")
	TArray<UPrimitiveComponent*> GetDetectablePrimitives() const { return DetectablePrimitives; }
	
	UFUNCTION(BlueprintPure, Category = "Interactable|Detection")
	int32 GetDetectionWeight() const {return DetectionWeight; }

	UFUNCTION(BlueprintCallable, Category = "Interactable|Detection")
	UE_API void AcquireInteractor(UFVInteractorComponent* NewInteractor);
	
	UFUNCTION(BlueprintCallable, Category = "Interactable|Detection")
	UE_API void ReleaseInteractor(UFVInteractorComponent* InteractorToRelease);

#pragma endregion 
	
	const FFVInteractionOffer* FindOffer(const FGameplayTag& InputTag) const;
	const TArray<FFVInteractionOffer>& GetOffers() const { return Offers; }

	UPROPERTY(BlueprintAssignable, Category = "Interactable|State")
	FInteractableFoundInteractor InteractorFound;
	
	UPROPERTY(BlueprintAssignable, Category = "Interactable|State")
	FInteractableLostInteractor InteractorLost;
	
	UPROPERTY(BlueprintAssignable, Category = "Interactable|State")
	FInteractableStateChanged StateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionStarted InteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionProgress InteractionProgressed;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionEnded InteractionEnded;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag InteractableType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FName DetectablePrimitiveTag = TEXT("Detectable");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Lifecycle", meta = (ClampMin = "0", Units = "s"))
	float CooldownPeriod;

protected:
	UPROPERTY(SaveGame, VisibleAnywhere, Category="MounteaInteraction|Read Only")
	TArray<TObjectPtr<UMeshComponent>> HighlightableComponents;

#pragma region Detection
	
	UPROPERTY(SaveGame, EditAnywhere, Category="Interactable|Detection", meta=(NoResetToDefault))
	TEnumAsByte<ECollisionChannel> CollisionChannel;
	
	UPROPERTY(SaveGame, EditAnywhere, Category="Interactable|Detection")
	FGameplayTagContainer CompatibleInteractorTags;
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category="Interactable|Detection")
	TArray<TObjectPtr<UPrimitiveComponent>>	DetectablePrimitives;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Detection", meta = (ClampMin = "-1"))
	int32 DetectionWeight;

#pragma endregion
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Actions", meta = (ForceInlineRow))
	TArray<FFVInteractionOffer> Offers;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	EFVInteractableState State;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	FGameplayTagContainer SuppressionReasons;

private:
	friend class UFVInteractorComponent;

	bool SetState(EFVInteractableState NewState);
	void StartInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor);
	void ProgressInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, float Progress);
	void EndInteraction(const FGameplayTag& ActionTag, UFVInteractorComponent* Interactor, const bool bSuccess);
	
	void ApplyStateTag(EFVInteractableState OldState, EFVInteractableState NewState) const;
	void StartCooldown();
	
	UPROPERTY(Transient)
	TObjectPtr<UFVInteractionRegistrySubsystem> Registry;

	UPROPERTY()
	TWeakObjectPtr<UFVInteractorComponent> TargetInteractor;

	UPROPERTY()
	FTimerHandle Timer_Interaction;
	
	UPROPERTY()
	FTimerHandle Timer_Cooldown;
	
	UPROPERTY()
	FTimerHandle Timer_ProgressExpiration;
};

#undef UE_API
