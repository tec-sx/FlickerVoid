// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionSignalComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionFocusChanged, UInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionOffersChanged, const TArray<FInteractionOffer>&, Offers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionRequested, const FInteractionCommit&, Commit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionStarted, const FInteractionCommit&, Commit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionProgress, const FInteractionCommit&, Commit, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionCancelled, const FInteractionCommit&, Commit, const FGameplayTag&, Reason);

#define UE_API FVINTERACTIONSYSTEM_API

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UInteractionSignalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionSignalComponent();

	static UE_API UInteractionSignalComponent* Get(const AActor* Actor);
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionFocusChanged FocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionOffersChanged OffersChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionRequested InteractionRequested;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionStarted InteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionProgress InteractionProgress;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FInteractionCancelled InteractionCancelled;
};

#undef UE_API