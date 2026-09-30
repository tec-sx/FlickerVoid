

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "FVInteractionSystemSettings.h"

#include "FVInteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UFVInteractionDebugSubsystem;
class UFVInteractableComponent;
class UFVInteractorResponseComponent;
class UFVInteractionRegistrySubsystem;
class UFVGestureComponent;

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
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void EnableTracing();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Detection")
	UE_API void DisableTracing();

	UFUNCTION(BlueprintPure, Category = "Interaction|Detection")
	UE_API bool HasInteractableTarget() const { return TargetInteractable != nullptr; }

	UFUNCTION(BlueprintPure)
	UE_API UFVInteractableComponent* GetTargetInteractable() const { return TargetInteractable.Get(); }
	
	UFUNCTION(BlueprintPure)
	UE_API const TArray<FFVInteractionOffer>& GetOffers() const { return CachedOffers; }

	UFUNCTION(BlueprintPure, Category = "Interaction|State")
	UE_API EFVInteractorState GetState() const { return State; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void AddSuppression(const FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interaction|State")
	UE_API void RemoveSuppression(const FGameplayTag Reason);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UE_API void RequestOfferRefresh() { RefreshOffers(); }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Responses")
	UE_API void BindResponse(UFVInteractorResponseComponent* Response);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Responses")
	UE_API void UnbindResponse(UFVInteractorResponseComponent* Response);

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity", meta = (Tooltip = "Tags this interactor presents to offer requirement gates."))
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Requirements", meta = (Tooltip = "Any offer whose ActionTag matches these is gated off entirely."))
	FGameplayTagContainer BlockedActionTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	UPROPERTY(EditAnywhere, Category="InteractorSettings")
	FGameplayTag InteractorTag;
	
	UPROPERTY(EditAnywhere, Category="DetectionSetup")
	TEnumAsByte<ECollisionChannel> OcclusionChannel;

	UPROPERTY(EditAnywhere, Category="DetectionSetup", meta=(UIMin=0, ClampMin=0, Units="cm"))
	float TraceRadius;
	
	UPROPERTY(EditAnywhere, Category="DetectionSetup", meta=(UIMin=0.01, ClampMin=0.01, Units="s"))
	float TickInterval;
	
	UPROPERTY(EditAnywhere, Category="DetectionSetup", meta=(UIMin=1, ClampMin=1, Units="cm"))
	float TraceRange;
	
	UPROPERTY(EditAnywhere, Category="DetectionSetup", meta=(NoResetToDefault, DisplayThumbnail=false))
	TArray<TObjectPtr<AActor>> IgnoredActors;

private:
	void SetState(const EFVInteractorState NewState);
	void PerformTrace();
	bool PerformOcclusionTest(const FVector& Start, const FVector& End, const AActor* Target) const;
	void ReleaseTargetInteractable();
	void RefreshOffers(bool bForceBroadcast = true);
	bool InteractableIsInReach(const UFVInteractableComponent* Target) const;
	bool IsOfferAvailable(const FFVInteractionOffer& Offer) const;
	
	bool BeginInteraction(const FGameplayTag InputTag);
	void TickInteraction(const float DeltaTime);
	bool ValidateActiveInteraction() const;
	void FinishInteraction(const bool bSuccess);
	const FFVInteractionOffer* FindOffer(const FGameplayTag& InputTag) const;
	UFVGestureComponent* ResolveGestureComponent();

	UPROPERTY(Transient)
	TWeakObjectPtr<UFVGestureComponent> GestureComponent;

	UPROPERTY(Transient)
	FFVInteractionCommit ActiveCommit;

	UPROPERTY(Transient)
	TWeakObjectPtr<UFVInteractableComponent> TargetInteractable;

	FDelegateHandle FactChangedHandle;

	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);
	bool PassesOfferConditions(const FFVInteractionOffer& Offer) const;

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractionRegistrySubsystem> Registry;
	
	UPROPERTY(Transient)
	TArray<FFVInteractionOffer> CachedOffers;
	
	EFVInteractorState State;
	FGameplayTagContainer SuppressionReasons;
	
#if !UE_BUILD_SHIPPING
	TWeakObjectPtr<UFVInteractionDebugSubsystem> DebugSubsystem;
#endif
};

#undef UE_API