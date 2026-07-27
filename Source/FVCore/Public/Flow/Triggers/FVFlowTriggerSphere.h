// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVFlowTriggerBase.h"
#include "MyClass.generated.h"

UCLASS()
class FLICKERVOIDCORE_API AMyClass : public AFVFlowTriggerBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
