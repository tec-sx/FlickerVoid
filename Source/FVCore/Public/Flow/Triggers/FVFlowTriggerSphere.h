// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVFlowTriggerBase.h"
#include "FVFlowTriggerSphere.generated.h"

UCLASS()
class FLICKERVOIDCORE_API AFVFlowTriggerSphere : public AFVFlowTriggerBase
{
	GENERATED_BODY()
public:
	explicit AFVFlowTriggerSphere(const FObjectInitializer& ObjectInitializer);
	
#if WITH_EDITOR
	//~ Begin AActor Interface.
	virtual void EditorApplyScale(
		const FVector& DeltaScale, 
		const FVector* PivotLocation, 
		bool bAltDown, 
		bool bShiftDown, 
		bool bCtrlDown) override;
	//~ End AActor Interface.
#endif
};
