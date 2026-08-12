#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactions/FVInteractionTypes.h"

#include "FVInteractionInstigatorComponent.generated.h"

class UFVInteractionTargetComponent;

//~=============================================================================
// Player-side interaction manager.
//
// Responsibilities:
//   - Track candidate targets registered via zone overlaps
//   - Broadcast focus changes to the UI
//   - Route input tag calls (E press, F hold, etc.) to the focused interactable
//~=============================================================================

UCLASS(Blueprintable, ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionInstigatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionInstigatorComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, 
		ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"), Category = Interaction)
	float DetectionUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"), Category = Interaction)
	float StickyFocusBonus = 0.1f;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionFocusChanged OnFocusChanged;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasFocus() const { return FocusedTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UFVInteractionTargetComponent* GetFocusedTarget() const { return FocusedTarget.Get(); }

	void RegisterCandidate(UFVInteractionTargetComponent* Target);
	void UnregisterCandidate(UFVInteractionTargetComponent* Target);

#if !UE_BUILD_SHIPPING
	const TArray<TWeakObjectPtr<UFVInteractionTargetComponent>>& GetDebugCandidates() const { return Candidates; }
#endif

private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> Owner;

	bool bIsInitialized = false;

	mutable TWeakObjectPtr<UFVInteractionTargetComponent> FocusedTarget;
	float TimeSinceLastUpdate = 0.f;

	TArray<TWeakObjectPtr<UFVInteractionTargetComponent>> Candidates;

	void DetectInteractables();
	UFVInteractionTargetComponent* FindBestTarget() const;
	void RefreshTickState();
};

