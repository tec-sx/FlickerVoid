// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FVInteractionComponent.generated.h"

class UFVInteractionActionData;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, AActor*, FocusedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionActionsChanged, AActor*, FocusedActor, const TArray<UFVInteractionActionData*>&, Actions);

/**
 * Component that handles interaction detection and management.
 * Performs traces to find interactables and manages focus states.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLICKERVOID_API UFVInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFVInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Called when an interactable is focused */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnFocusChanged OnFocusGained;

	/** Called when focus is lost */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnFocusChanged OnFocusLost;

	/** Called when available actions change */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionActionsChanged OnActionsChanged;

	/** Get the currently focused actor */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetFocusedActor() const { return CurrentFocusedActor; }

	/** Get available actions for the current focused actor */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	TArray<UFVInteractionActionData*> GetCurrentActions() const { return CurrentActions; }

	/** Attempt to execute an interaction action */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool ExecuteAction(UFVInteractionActionData* Action);

	/** Start holding an action (for hold-type interactions) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartHoldingAction(UFVInteractionActionData* Action);

	/** Stop holding an action */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopHoldingAction();

	/** Get hold progress (0-1) for current hold action */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	float GetHoldProgress() const;

protected:
	virtual void BeginPlay() override;

	/** Perform trace to find interactables */
	void PerformInteractionTrace();

	/** Update the focused actor */
	void UpdateFocusedActor(AActor* NewActor);

	/** Update available actions */
	void UpdateAvailableActions();

private:
	/** Maximum distance for interactions */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float InteractionDistance;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float TraceRadius;

	/** Trace channel to use for interaction detection */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> TraceChannel;

	/** How often to perform interaction traces (per second) */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float TraceFrequency;

	/** Currently focused actor */
	UPROPERTY()
	TObjectPtr<AActor> CurrentFocusedActor;

	/** Available actions for current focused actor */
	UPROPERTY()
	TArray<TObjectPtr<UFVInteractionActionData>> CurrentActions;

	/** Action currently being held */
	UPROPERTY()
	TObjectPtr<UFVInteractionActionData> CurrentHoldAction;

	/** Time spent holding current action */
	float HoldTimeAccumulator;

	/** Time since last trace */
	float TraceCooldown;
};
