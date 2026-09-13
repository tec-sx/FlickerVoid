

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "FVInteractionSystemSettings.h"

#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "FVInteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractorStateChanged, EFVInteractorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionFocusChanged, UFVInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionOffersChanged, const TArray<FFVInteractionOffer>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionCommitStarted, const FFVInteractionCommit&, Commit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionCommitProgress, const FFVInteractionCommit&, Commit, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionCommitEnded, const FFVInteractionCommit&, Commit, const bool, bSuccess);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UFVInteractorComponent final : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFVInteractorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure)
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure)
	UE_API UFVInteractableComponent* GetFocusedTarget() const { return FocusedTarget.Get(); }
	
	UFUNCTION(BlueprintPure)
	UE_API const TArray<FFVInteractionOffer>& GetOffers() const { return CachedOffers; }
	
	UFUNCTION(BlueprintCallable)
	UE_API bool PushInput(FGameplayTag InputTag, EFVInteractionInputPhase Phase);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void EnableTracing();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void DisableTracing();

	UFUNCTION(BlueprintPure, Category = "Interaction|Detection")
	UE_API bool IsTracing() const { return bIsTracing; }

	UFUNCTION(BlueprintPure, Category = "Interaction|State")
	UE_API EFVInteractorState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void AddSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void RemoveSuppression(FGameplayTag Reason);

	UFUNCTION(BlueprintPure, Category = "Interaction|State")
	UE_API bool IsSuppressed() const { return !SuppressionReasons.IsEmpty(); }

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractorStateChanged StateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionFocusChanged FocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionOffersChanged OffersChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionCommitStarted InteractionCommitStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionCommitProgress InteractionCommitProgress;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionCommitEnded InteractionCommitEnded;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity", meta = (Tooltip = "Tags this interactor presents to offer requirement gates."))
	FGameplayTagContainer InteractorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Requirements", meta = (Tooltip = "Any offer whose ActionTag matches these is gated off entirely."))
	FGameplayTagContainer BlockedActionTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	EFVInteractableDetectionMode Precision = EFVInteractableDetectionMode::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ShowOnlyInnerProperties))
	FFVDetectionSetup TracingSetup;

	UFUNCTION(BlueprintPure, Category = "Interaction|Detection")
	UE_API FVector GetDetectionOrigin() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UE_API void RequestOfferRefresh() { RefreshOffers(); }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Identity")
	UE_API void AddInteractorTag(FGameplayTag NewTag);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Identity")
	UE_API void RemoveInteractorTag(FGameplayTag OldTag);

#if !UE_BUILD_SHIPPING
	enum class EDebugActionOutcome : uint8
	{
		None,
		Succeeded,
		NoPrompt,
		Disabled,
	};

	const TArray<UFVInteractableComponent*>& GetDebugCandidates() const { return Candidates; }
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
	bool PerformSafetyTrace(const FVector& Origin, const UFVInteractableComponent& Candidate) const;
	void SetFocusedTarget(UFVInteractableComponent* NewTarget);

	bool BeginInteraction(const FFVInteractionOffer& Offer, UFVInteractableComponent& Target);
	void TickInteraction();
	void CommitInteraction();
	void ProgressInteraction(const float Progress);
	void CancelInteraction(const FGameplayTag& Reason);
	const FFVInteractionOffer* FindActiveOffer() const;

	UFUNCTION()
	void HandleInRangeSetChanged(bool bHasAnyInRange);

	UFUNCTION()
	void HandleFocusedStateChanged(EFVInteractableState NewState);

	bool IsOfferAvailable(const FFVInteractionOffer& Offer) const;

	UPROPERTY(Transient)
	TObjectPtr<APawn> Owner;

	bool bIsInitialized = false;
	bool bIsTracing = false;
	mutable TWeakObjectPtr<UFVInteractableComponent> FocusedTarget;

	UFVInteractionRegistrySubsystem* Registry;
	TArray<UFVInteractableComponent*> Candidates;
	FVector LastFocusImpactPoint = FVector::ZeroVector;
	FTimerHandle TraceTimer;

	FTimerHandle InteractionTimer;
	FFVInteractionCommit ActiveCommit;
	EFVInteractionInputMode ActiveMode = EFVInteractionInputMode::Default;
	float ActiveDuration = 0.f;
	float ActiveElapsed = 0.f;
	float LastProgressBroadcast = 0.f;
	int32 ActivePresses = 0;
	int32 ActiveRequiredPresses = 0;
	bool bIsInteracting = false;

	void SetState(EFVInteractorState NewState);
	void UpdateState();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction|State", meta = (AllowPrivateAccess = "true"))
	EFVInteractorState State = EFVInteractorState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction|State", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer SuppressionReasons;

	UPROPERTY(Transient)
	TArray<FFVInteractionOffer> CachedOffers;

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