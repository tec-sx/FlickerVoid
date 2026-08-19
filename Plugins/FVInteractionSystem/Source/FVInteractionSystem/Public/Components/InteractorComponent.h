

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"

#include "InteractorComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;


UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"), Category = "Interaction")
	float DetectionUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"), Category = "Interaction")
	float StickyFocusBonus = 0.1f;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionFocusChanged OnFocusChanged;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UInteractableComponent* GetFocusedTarget() const { return FocusedTarget.Get(); }

	void RegisterCandidate(UInteractableComponent* Target);
	void UnregisterCandidate(UInteractableComponent* Target);

#if !UE_BUILD_SHIPPING
	const TArray<TWeakObjectPtr<UInteractableComponent>>& GetDebugCandidates() const { return Candidates; }
#endif

private:
	UPROPERTY(Transient)
	TObjectPtr<APawn> Owner;

	bool bIsInitialized = false;

	mutable TWeakObjectPtr<UInteractableComponent> FocusedTarget;
	float TimeSinceLastUpdate = 0.f;

	TArray<TWeakObjectPtr<UInteractableComponent>> Candidates;

	void DetectInteractables();
	UInteractableComponent* FindBestTarget() const;
};

#undef UE_API