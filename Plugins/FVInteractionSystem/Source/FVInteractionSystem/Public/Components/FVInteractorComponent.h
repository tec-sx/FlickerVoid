

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "Data/FVInteractorModeDefinition.h"
#include "FVInteractionSystemSettings.h"

#include "FVInteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractionDebugSubsystem;
class UFVInteractableComponent;
class UFVInteractionRegistrySubsystem;
class UFVGestureComponent;
class UFVInteractorDefinition;

USTRUCT()
struct FFVInteractorModeEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UFVInteractorModeDefinition> Mode;

	int32 Priority = 0;
	uint32 PushOrder = 0;
};

struct FTraceData
{
	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	FRotator TraceRotation = FRotator::ZeroRotator;
	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	ECollisionChannel CollisionChannel = ECC_Visibility;
	TArray<FHitResult> HitResults;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractorStateChanged, EFVInteractorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractorFoundInteractable, UFVInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractorLostInteractable, UFVInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionOffersChanged, const TArray<FFVInteractionOfferData>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionCommitStarted, const FFVInteractionCommit&, Commit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionCommitProgress, const FFVInteractionCommit&, Commit, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionCommitEnded, const FFVInteractionCommit&, Commit, const bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractorModeChanged, FGameplayTag, NewMode, FGameplayTag, OldMode);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UFVInteractorComponent final : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFVInteractorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Interaction|Mode")
	UE_API bool PushMode(const FGameplayTag ModeTag, const int32 Priority = 0);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Mode")
	UE_API bool RemoveMode(const FGameplayTag ModeTag);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Mode")
	UE_API void ClearModes();

	UFUNCTION(BlueprintPure, Category = "Interaction|Mode")
	UE_API bool HasMode(const FGameplayTag ModeTag) const;

	UFUNCTION(BlueprintPure, Category = "Interaction|Mode")
	UE_API FGameplayTag GetActiveModeTag() const;

	UFUNCTION(BlueprintPure, Category = "Interaction|Mode")
	const FFVInteractorDetectionSettings& GetActiveDetection() const { return ActiveDetection; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Identity")
	UE_API FGameplayTag GetInteractorTag() const;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Mode")
	FInteractorModeChanged InteractorModeChanged;

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void EnableTracing();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void DisableTracing();

	UFUNCTION(BlueprintPure, Category = "Interaction|Detection")
	UE_API bool HasInteractableTarget() const { return TargetInteractable != nullptr; }

	UFUNCTION(BlueprintPure)
	UE_API UFVInteractableComponent* GetTargetInteractable() const { return TargetInteractable.Get(); }
	
	UFUNCTION(BlueprintPure)
	UE_API const TArray<FFVInteractionOfferData>& GetOffers() const { return CachedOffers; }

	UFUNCTION(BlueprintPure, Category = "Interaction|State")
	UE_API EFVInteractorState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void AddSuppression(const FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void RemoveSuppression(const FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UE_API void RequestOfferRefresh() { RefreshOffers(); }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Identity")
	UE_API void GrantTag(const FGameplayTag NewTag);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Identity")
	UE_API void RemoveTag(const FGameplayTag OldTag);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction|Input")
	UE_API bool PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Input")
	UE_API void CancelInteraction(const FGameplayTag Reason);
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractorStateChanged StateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractorFoundInteractable InteractableFound;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractorLostInteractable InteractableLost;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionOffersChanged OffersChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionCommitStarted InteractionCommitStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionCommitProgress InteractionCommitProgressed;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|State")
	FInteractionCommitEnded InteractionCommitEnded;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UFVInteractorDefinition> Definition;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Interaction|Identity", meta = (Tooltip = "Runtime tags this interactor presents to offer conditions. Initialized from Definition."))
	FGameplayTagContainer GrantedTags;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Interaction|Requirements", meta = (Tooltip = "Runtime blocked action tags. Initialized from Definition."))
	FGameplayTagContainer BlockedActionTags;

	UPROPERTY(EditAnywhere, Category="Interaction|Detection", meta=(NoResetToDefault, DisplayThumbnail=false))
	TArray<TObjectPtr<AActor>> IgnoredActors;

private:
	void ApplyTopMode();
	void TickModeBlend(const float DeltaTime);
	const FFVInteractorModeEntry* GetTopEntry() const;

	UPROPERTY(Transient)
	TArray<FFVInteractorModeEntry> ModeStack;

	UPROPERTY(Transient)
	TObjectPtr<const UFVInteractorModeDefinition> ActiveMode;

	FFVInteractorDetectionSettings ActiveDetection;
	FFVInteractorDetectionSettings BlendFrom;
	FFVInteractorDetectionSettings BlendTarget;
	float BlendElapsed = 0.f;
	float BlendDuration = 0.f;
	uint32 NextPushOrder = 0;

	void SetState(const EFVInteractorState NewState);
	void PerformTrace();
	float ScoreCandidate(const UFVInteractableComponent* Candidate, const FHitResult& Hit, const FVector& ViewLocation, const FVector& ViewDirection) const;
	bool PerformOcclusionTest(const FVector& Start, const FVector& End, const AActor* Target) const;
	void ReleaseTargetInteractable();
	void RefreshOffers(bool bForceBroadcast = true);
	bool InteractableIsInReach(const UFVInteractableComponent* Target) const;
	bool IsOfferAvailable(const FFVInteractionOffer& Offer) const;
	
	bool BeginInteraction(const FGameplayTag InputTag);
	void TickInteraction(const float DeltaTime);
	bool ValidateActiveInteraction() const;
	void FinishInteraction(const bool bSuccess);
	const FFVInteractionOfferData* FindOffer(const FGameplayTag& InputTag) const;
	UFVGestureComponent* ResolveGestureComponent();

	UPROPERTY(Transient)
	TWeakObjectPtr<UFVGestureComponent> GestureComponent;

	UPROPERTY(Transient)
	FFVInteractionCommit ActiveCommit;

	UPROPERTY(Transient)
	TWeakObjectPtr<UFVInteractableComponent> TargetInteractable;

	FDelegateHandle FactChangedHandle;

	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractionRegistrySubsystem> Registry;
	
	UPROPERTY(Transient)
	TArray<FFVInteractionOfferData> CachedOffers;
	
	EFVInteractorState State;
	FGameplayTagContainer SuppressionReasons;
	
#if !UE_BUILD_SHIPPING
	TWeakObjectPtr<UFVInteractionDebugSubsystem> DebugSubsystem;
#endif
};

#undef UE_API
