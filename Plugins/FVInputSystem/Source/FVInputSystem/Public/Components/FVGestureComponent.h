// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/FVInputTypes.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"

#include "FVGestureComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FVINPUTSYSTEM_API UFVGestureComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVGestureComponent();
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	bool BeginGesture(const FGameplayTag InInputTag, const FFVGesture& InGesture);
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	bool PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase);
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	EFVGestureStatus UpdateGesture(const float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	void ResetGesture();
	
	UFUNCTION(BlueprintPure, Category="Gestures")
	float GetProgress() const { return Progress; }
	
	const FFVGesture* GetActiveGesture() const {return ActiveGesture.GetPtrOrNull(); }
	
	UPROPERTY(EditAnywhere, Category="Gestures", meta=(ToolTip="Measure gestures in real time."))
	bool bIgnoreTimeDilation;
	
	UPROPERTY(EditAnywhere, DisplayName="DecayPerSecond", Category="Gestures|Mash", meta=(ClampMin="0", ToolTip="Presses lost per second."))
	float MashDecayPerSecond;
	
private:
	EFVGestureStatus EvaluatePress();
	EFVGestureStatus EvaluateHold();
	EFVGestureStatus EvaluateMash(const float DeltaTime);
	
	FGameplayTag ActiveInputTag;
	TOptional<FFVGesture> ActiveGesture;
	
	float Elapsed = 0.f;
	float Progress = 0.f;
	float Presses = 0.f;
	bool bReleased = false;
	bool bCanceled = false;
};
