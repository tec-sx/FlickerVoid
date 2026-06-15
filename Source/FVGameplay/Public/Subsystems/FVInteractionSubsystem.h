// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactions/FVInteractionTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "FVInteractionSubsystem.generated.h"

struct FFVInteractionActionInfo;
class UFVInteractionAction;
class UFVInteractionInstigatorComponent;
class UFVInteractionTargetComponent;
/**
 * 
 */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UFVInteractionInstigatorComponent* GetInstigator() const;
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UFVInteractionTargetComponent* GetFocusedTarget() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasFocus() const { return FocusedTarget.IsValid(); }
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	TArray<UFVInteractionAction*> GetAvailableActions() const;
	
	UFUNCTION(BlueprintPure, Category = "UI")
	TArray<FFVInteractionActionInfo> GetAvailableActionsUIInfo() const;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|UI")
	FOnInteractionFocusChanged OnFocusChanged;
	
	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);
	void UpdateFocus(UFVInteractionTargetComponent* NewTarget);

private:
	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<UFVInteractionInstigatorComponent> Instigator;
	TWeakObjectPtr<UFVInteractionTargetComponent> FocusedTarget;
};
