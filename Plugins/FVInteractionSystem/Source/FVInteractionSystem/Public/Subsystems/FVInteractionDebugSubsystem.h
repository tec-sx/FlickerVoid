#pragma once

#include "GameplayTagContainer.h"
#include "Core/FVInteractionTypes.h"

#include "FVInteractionDebugSubsystem.generated.h"

class UFVInteractableComponent;
class UFVInteractorComponent;
struct FTraceData;

enum class EFVDebugInteractionOutcome : uint8
{
	None,
	Succeeded,
	NoPrompt,
	Disabled,
};

UCLASS()
class FVINTERACTIONSYSTEM_API UFVInteractionDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

#if !UE_BUILD_SHIPPING
	void Register(const TWeakObjectPtr<UFVInteractorComponent> InInteractor);
	void Unregister();
	
	static void VisualizeRange(const UWorld* World, const UFVInteractorComponent* Interactor, const float Radius, TArray<TObjectPtr<UFVInteractableComponent>>& ActiveInteractables, const float Interval);
	static void VisualizeTrace(const UWorld* World, const FTraceData& InTraceData, const float TraceRadius, const float Interval);
	
	void DebugTrace(const TArray<FHitResult>& InHitResults);
	void DebugOcclusion(const FHitResult& HitResult);
	void DebugInput(const FGameplayTag& InputTag, const double InputTime);
	void DebugInteractionOutcome(const EFVDebugInteractionOutcome& Outcome);
	
private:
	void DrawHUD(UCanvas* Canvas, APlayerController* PC);
	
	FDelegateHandle HUDDrawHandle;

	// Detection
	TWeakObjectPtr<UFVInteractorComponent> InteractorPtr;
	TArray<FHitResult> HitResults;
	FHitResult OcclusionHitResult;
	
	// Input
	FGameplayTag LastInputTag;
	double LastInputTime;
	
	// Interaction
	TArray<FFVInteractionOffer> AvailableOffers;
	FFVInteractionCommit ActiveCommit;
	float InteractionProgress;
	EFVDebugInteractionOutcome LastInteractionOutcome;
	
#endif
	
private:
	UFUNCTION()
	void OnOffersChanged(const TArray<FFVInteractionOffer> Offers);
	
	UFUNCTION()
	void OnInteractionCommitStarted(const FFVInteractionCommit& Commit);
	
	UFUNCTION()
	void OnInteractionProgressed(const FFVInteractionCommit& Commit, float Progress);
};
