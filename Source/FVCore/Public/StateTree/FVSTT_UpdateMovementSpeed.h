// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "FVSTT_UpdateMovementSpeed.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, Category = "Flicker Void", meta = (DisplayName = "Update Movement Speed"))
class FLICKERVOIDCORE_API UFVSTT_UpdateMovementSpeed : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	TObjectPtr<AActor> Actor;
	
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	TObjectPtr<AController> Controller;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float TargetMaxWalkSpeed = 600.0f;	

#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID, 
		FStateTreeDataView InstanceDataView, 
		const IStateTreeBindingLookup& BindingLookup, 
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
	
protected:
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context, 
		const FStateTreeTransitionResult& Transition) override;
};
