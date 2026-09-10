

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"
#include "FVInteractionSystemSettings.h"

#include "Subsystems/InteractionRegistrySubsystem.h"
#include "InteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractorStateChanged, EInteractorState, NewState);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UInteractorComponent final : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure)
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure)
	UE_API UInteractableComponent* GetFocusedTarget() const { return FocusedTarget.Get(); }
	
	UFUNCTION(BlueprintPure)
	UE_API const TArray<FInteractionOffer>& GetOffers() const { return CachedOffers; }
	
	UFUNCTION(BlueprintCallable)
	UE_API bool PushInput(FGameplayTag InputTag, EInteractionInputPhase Phase);

	UFUNCTION(BlueprintCallable)
	UE_API bool TryExecuteInteraction(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void EnableTracing();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void DisableTracing();

	UFUNCTION(BlueprintPure, Category = "Interaction|Detection")
	UE_API bool IsTracing() const { return bIsTracing; }

	UFUNCTION(BlueprintPure, Category = "Interaction|State")
	UE_API EInteractorState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void AddSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void RemoveSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintPure, Category = "Interaction|State")
	UE_API bool IsSuppressed() const { return !SuppressionReasons.IsEmpty(); }

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FOnInteractorStateChanged OnStateChanged;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity", meta = (Tooltip = "Tags this interactor presents to offer requirement gates."))
	FGameplayTagContainer InteractorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Requirements", meta = (Tooltip = "Any offer whose ActionTag matches these is gated off entirely."))
	FGameplayTagContainer BlockedActionTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	EInteractorPrecision Precision = EInteractorPrecision::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ShowOnlyInnerProperties))
	FTracingSetup TracingSetup;

	UFUNCTION(BlueprintPure, Category = "Interaction|Detection")
	UE_API FVector GetDetectionOrigin() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UE_API void RequestOfferRefresh() { RefreshOffers(); }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Identity")
	UE_API void AddInteractorTag(FGameplayTag NewTag);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Identity")
	UE_API void RemoveInteractorTag(FGameplayTag OldTag);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Events")
	UE_API void ReportInteractionProgress(const FInteractionCommit& Commit, float Progress);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Events")
	UE_API void ReportInteractionCancelled(const FInteractionCommit& Commit, const FGameplayTag& Reason);

#if !UE_BUILD_SHIPPING
	enum class EDebugActionOutcome : uint8
	{
		None,
		Succeeded,
		NoPrompt,
		Disabled,
	};

	const TArray<UInteractableComponent*>& GetDebugCandidates() const { return Candidates; }
	float GetDebugTraceRange() const { return TracingSetup.TracingRange; }
	EDebugActionOutcome GetDebugLastOutcome() const { return DebugLastOutcome; }
	FGameplayTag GetDebugLastInputTag() const { return DebugLastInputTag; }
	double GetDebugLastActionTime() const { return DebugLastActionTime; }
	bool IsDebugGateOpen() const { return bDebugGateOpen; }
	bool DidDebugHitOccluder() const { return bDebugHitOccluder; }
	bool HasDebugImpact() const { return bDebugHasImpact; }
	FVector GetDebugImpactPoint() const { return DebugImpactPoint; }
	FVector GetDebugSweepDirection() const { return DebugSweepDirection; }
#endif

private:
	void RefreshOffers(bool bForceBroadcast = true);
	void ProcessTrace();
	void ArmNextTrace();
	bool GetTraceOrigin(FVector& OutOrigin, FVector& OutForward) const;
	bool PerformSafetyTrace(const FVector& Origin, const UInteractableComponent& Candidate) const;
	void SetFocusedTarget(UInteractableComponent* NewTarget);

	bool BeginInteraction(const FInteractionOffer& Offer, UInteractableComponent& Target);
	void TickInteraction();
	void CommitInteraction();
	void CancelInteraction(const FGameplayTag& Reason);
	const FInteractionOffer* FindActiveOffer() const;

	UFUNCTION()
	void HandleInRangeSetChanged(bool bHasAnyInRange);

	UFUNCTION()
	void HandleFocusedStateChanged(EInteractableState NewState);

	bool IsOfferAvailable(const FInteractionOffer& Offer) const;

	UPROPERTY(Transient)
	TObjectPtr<APawn> Owner;

	bool bIsInitialized = false;
	bool bIsTracing = false;
	mutable TWeakObjectPtr<UInteractableComponent> FocusedTarget;

	UInteractionRegistrySubsystem* Registry;
	TArray<UInteractableComponent*> Candidates;
	FVector LastFocusImpactPoint = FVector::ZeroVector;
	FTimerHandle TraceTimer;

	FTimerHandle InteractionTimer;
	FInteractionCommit ActiveCommit;
	EInteractionInputMode ActiveMode = EInteractionInputMode::Default;
	float ActiveDuration = 0.f;
	float ActiveElapsed = 0.f;
	float LastProgressBroadcast = 0.f;
	int32 ActivePresses = 0;
	int32 ActiveRequiredPresses = 0;
	bool bIsInteracting = false;

	void SetState(EInteractorState NewState);
	void UpdateState();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction|State", meta = (AllowPrivateAccess = "true"))
	EInteractorState State = EInteractorState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction|State", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer SuppressionReasons;

	UPROPERTY(Transient)
	TArray<FInteractionOffer> CachedOffers;

#if !UE_BUILD_SHIPPING
	EDebugActionOutcome DebugLastOutcome = EDebugActionOutcome::None;
	FGameplayTag DebugLastInputTag;
	double DebugLastActionTime = 0.0;
	FVector DebugImpactPoint = FVector::ZeroVector;
	FVector DebugSweepDirection = FVector::ForwardVector;
	bool bDebugHitOccluder = false;
	bool bDebugHasImpact = false;
	bool bDebugGateOpen = false;
#endif
};

#undef UE_API