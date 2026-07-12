// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVSTE_AICharacter.h"

void UFVSTE_AICharacter::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);
}

void UFVSTE_AICharacter::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);
	CurrentHealth = 0.f;
}

void UFVSTE_AICharacter::TreeStop(FStateTreeExecutionContext& Context)
{
	Super::TreeStop(Context);
}
