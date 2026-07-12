// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeAIComponent.h"
#include "FVStateTreeAIComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLICKERVOIDAI_API UFVStateTreeAIComponent : public UStateTreeAIComponent
{
	GENERATED_BODY()

public:
	UFVStateTreeAIComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void ValidateStateTreeReference() override;
	
	UFUNCTION(BlueprintCallable, Category = "StateTree", meta = (AutoCreateRefTerm = "StateTreeReference", DisplayName = "Start State Tree"))
	void StartStateTree(UStateTree* StateTree);
	
	UFUNCTION(BlueprintCallable, Category = "StateTree", meta = (DisplayName = "Stop State Tree"))
	void StopStateTree();
	
	UFUNCTION(BlueprintCallable, Category = "StateTree", meta = (AutoCreateRefTerm = "EventTag", DisplayName = "Send Event"))
	void SendEvent(const FGameplayTag& EventTag);

private:
	FGameplayTag LastEventTag = FGameplayTag::EmptyTag;
};
