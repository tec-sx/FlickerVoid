// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVCoreInputTypes.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FVGestureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVGestureProgressed, FGameplayTag, InputTag, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVGestureEnded, FGameplayTag, InputTag, bool, bSuccess);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLICKERVOID_API UFVGestureComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVGestureComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	bool BeginGesture(const FFVGesture& InGesture);
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	bool PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase);
	
	UFUNCTION(BlueprintCallable, Category="Gestures")
	void AbortGesture();

	UPROPERTY(BlueprintAssignable, Category="Gestures")
	FFVGestureProgressed GestureProgressed;
	
	UPROPERTY(BlueprintAssignable, Category="Gestures")
	FFVGestureEnded GestureEnded;
	
	UPROPERTY(EditAnywhere, Category="Gestures", meta=(ToolTip="Measure gestures in real time."))
	bool bIgnoreTimeDilation;
	
	UPROPERTY(EditAnywhere, DisplayName="DecayPerSecond", Category="Gestures|Mash", meta=(ClampMin="1", ToolTip="Presses lost per second."))
	float MashDecayPerSecond;
	
private:
	void HandleInput(const EFVInputPhase Phase);
	void EndGesture(const bool bSuccess);
	
	EFVGestureStatus ProgressGesture(const float DeltaTime);
	EFVGestureStatus ProgressPress();
	EFVGestureStatus ProgressHold(const float DeltaTime);
	EFVGestureStatus ProgressMash(const float DeltaTime);
	
	TOptional<FFVGesture> ActiveGesture;
	
	float Elapsed = 0.f;
	float Progress = 0.f;
	float Presses = 0.f;
	bool bHoldReleased = false;
	bool bMashCanceled = false;
};
