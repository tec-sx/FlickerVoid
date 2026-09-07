

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionRequirement.h"
#include "Core/InteractionTypes.h"

#include "Subsystems/InteractionRegistrySubsystem.h"
#include "InteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;
class UInteractionRequirement;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FExecuteInteractionAction, const FGameplayTag& /*ActionTag*/, const FInteractionContext& /*Context*/);

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintPure)
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure)
	UE_API UInteractableComponent* GetFocusedTarget() const { return FocusedTarget.Get(); }
	
	UFUNCTION(BlueprintPure)
	UE_API const TArray<FInteraction>& GetPrompts() const { return CachedInteractions; }
	
	UFUNCTION(BlueprintCallable)
	UE_API bool TryExecuteInteraction(FGameplayTag InputTag);
	
	UFUNCTION(BlueprintCallable)
	UE_API void SetEnabled(bool Value) { bEnabled = Value; }
	
	UE_API bool GetAimPoint(FVector& OutOrigin, FVector& OutForward) const;

	FExecuteInteractionAction ExecuteAction;

	UPROPERTY(EditAnywhere, Instanced, Category = "Interaction")
	TArray<TObjectPtr<UInteractionRequirement>> GlobalRequirements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	FVector AimOriginOffset = FVector(0.f, 0.f, 60.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float MaxDetectionRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DetectionUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ClampMin = "0"))
	float AimSweepRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Detection")
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_GameTraceChannel1;

	UPROPERTY(BlueprintAssignable)
	FOnInteractionFocusChanged OnFocusChanged;

	UPROPERTY(BlueprintAssignable)
	FOnInteractionOffersChanged OnOffersChanged;

#if !UE_BUILD_SHIPPING
	enum class EDebugActionOutcome : uint8
	{
		None,
		Succeeded,
		NoPrompt,
		Disabled,
		ExecuteFailed,
	};

	const TArray<UInteractableComponent*>& GetDebugCandidates() const { return Candidates; }
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
	void DetectInteractables();
	void SetFocusedTarget(UInteractableComponent* NewTarget);
	FInteractionContext MakeContext(const UInteractableComponent& Target) const;
	bool EvaluateRequirements(
		const FGameplayTag ActionTag, 
		const TArray<TObjectPtr<UInteractionRequirement>>& Requirements,
		bool& bOutHidden) const;

	UPROPERTY(Transient)
	TObjectPtr<APawn> Owner;

	bool bIsInitialized = false;
	bool bEnabled = true;
	mutable TWeakObjectPtr<UInteractableComponent> FocusedTarget;
	float TimeSinceLastUpdate = 0.f;

	UInteractionRegistrySubsystem* Registry;
	TArray<UInteractableComponent*> Candidates;
	FVector LastFocusImpactPoint = FVector::ZeroVector;

	UPROPERTY(Transient)
	TArray<FInteraction> CachedInteractions;

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