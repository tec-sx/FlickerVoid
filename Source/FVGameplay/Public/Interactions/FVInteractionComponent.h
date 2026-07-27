// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ArrowComponent.h"
#include "UFVInteractionComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FFVPlayerInInteractionEvent, TWeakObjectPtr<class UInteractionComponent> /*Interaction*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVInteractionComponentEvent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UUFVInteractionComponent : public UArrowComponent
{
	GENERATED_BODY()
	
	static FFVPlayerInInteractionEvent OnPlayerEnter;
	static FFVPlayerInInteractionEvent OnPlayerExit;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = 50.0f))
	float Distance;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FFVInteractionComponentEvent OnUsed;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Enable();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Disable();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bEnabled;

private:
	bool bCanInteract;
	TWeakObjectPtr<APlayerCameraManager> CameraManager;
	
};
