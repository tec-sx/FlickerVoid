

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"

#include "Subsystems/InteractionRegistrySubsystem.h"
#include "InteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;

/**
 * Seam to the owning game's ability system. The plugin never talks to GAS directly:
 * the game binds these on its pawn. While unbound every action reports Available and
 * execution is a no-op, so the plugin stays usable standalone.
 */
DECLARE_DELEGATE_RetVal_OneParam(FInteractionAvailabilityResult, FResolveInteractionAction, const FGameplayTag& /*ActionTag*/);
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
	UE_API void RefreshOffers();

	UFUNCTION(BlueprintCallable)
	UE_API EInteractionResult TryExecuteAction(FGameplayTag InputTag);

	/** Bound by the game to validate an action against its ability system. */
	FResolveInteractionAction ResolveAction;

	/** Bound by the game to actually run an action through its ability system. */
	FExecuteInteractionAction ExecuteAction;

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
	const TArray<UInteractableComponent*>& GetDebugCandidates() const { return Candidates; }
#endif

private:
	void DetectInteractables();
	void SetFocusedTarget(UInteractableComponent* NewTarget);
	void RevalidatePrompts();
	FInteractionAvailabilityResult ResolveAvailability(const FGameplayTag& ActionTag) const;
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
};

#undef UE_API