#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Core/InteractionTypes.h"

#include "InteractionResponseComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseFocusChanged, UInteractableComponent*, FocusedTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseOffersChanged, const TArray<FInteractionOffer>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseInteractionStarted, const FInteractionCommit&, Commit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResponseInteractionProgress, const FInteractionCommit&, Commit, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseInteractionRequested, const FInteractionCommit&, Commit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResponseInteractionCancelled, const FInteractionCommit&, Commit, const FGameplayTag&, Reason);

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UInteractionResponseComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionResponseComponent();

	UFUNCTION(BlueprintPure, Category = "Interaction|Response", meta = (DefaultToSelf = "Actor"))
	static UE_API UInteractionResponseComponent* Get(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Response")
	UE_API void ReportInteractionProgress(const FInteractionCommit& Commit, float Progress);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Response")
	UE_API void ReportInteractionCancelled(const FInteractionCommit& Commit, const FGameplayTag& Reason);

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Response")
	FOnResponseFocusChanged OnFocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Response")
	FOnResponseOffersChanged OnOffersChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Response")
	FOnResponseInteractionStarted OnInteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Response")
	FOnResponseInteractionProgress OnInteractionProgress;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Response")
	FOnResponseInteractionRequested OnInteractionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Response")
	FOnResponseInteractionCancelled OnInteractionCancelled;
};

#undef UE_API
