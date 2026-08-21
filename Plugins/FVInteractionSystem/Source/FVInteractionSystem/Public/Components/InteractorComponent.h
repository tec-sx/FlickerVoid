

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	UE_API const TArray<FInteractionPrompt>& GetPrompts() const { return CachedPrompts; }

	UFUNCTION(BlueprintCallable)
	UE_API bool TryExecuteAction(FGameplayTag InputTag);

	FExecuteInteractionAction ExecuteAction;

	UPROPERTY(EditAnywhere, Instanced, Category = "Interaction")
	TArray<TObjectPtr<UInteractionRequirement>> GlobalRequirements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float MaxDetectionRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DetectionUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float StickyFocusBonus = 0.1f;

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
#endif

private:
	void RefreshOffers(bool bForceBroadcast = true);
	void DetectInteractables();
	void SetFocusedTarget(UInteractableComponent* NewTarget);
	bool ResolveAvailability(const FInteractionOffer& Offer, bool& bOutHidden) const;
	FInteractionContext MakeContext(const UInteractableComponent& Target) const;

	UPROPERTY(Transient)
	TObjectPtr<APawn> Owner;

	bool bIsInitialized = false;
	mutable TWeakObjectPtr<UInteractableComponent> FocusedTarget;
	float TimeSinceLastUpdate = 0.f;

	UInteractionRegistrySubsystem* Registry;
	TArray<UInteractableComponent*> Candidates;

	UPROPERTY(Transient)
	TArray<FInteractionPrompt> CachedPrompts;

#if !UE_BUILD_SHIPPING
	EDebugActionOutcome DebugLastOutcome = EDebugActionOutcome::None;
	FGameplayTag DebugLastInputTag;
	double DebugLastActionTime = 0.0;
#endif
};

#undef UE_API