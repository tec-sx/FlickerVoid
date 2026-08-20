

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"

#include "Subsystems/InteractionRegistrySubsystem.h"
#include "InteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;
class UAbilitySystemComponent;

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
	bool HasAbilityForAction(const UAbilitySystemComponent& ASC, const FGameplayTag& ActionTag) const;
	bool CanActivateAction(const UAbilitySystemComponent& ASC, const FGameplayTag& ActionTag) const;
	UAbilitySystemComponent* GetOwnerASC() const;
	
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