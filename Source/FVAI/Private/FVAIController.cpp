// Fill out your copyright notice in the Description page of Project Settings.

#include "FVAIController.h"
#include "Character/FVCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAIController)

AFVAIController::AFVAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = false;
	bStartAILogicOnPossess = true;
}

void AFVAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedCharacter = Cast<AFVCharacter>(InPawn);
	
	if (CachedCharacter)
	{
		UE_LOG(LogTemp, Log, TEXT("AFVAIController: Possessed %s"), *CachedCharacter->GetName());
	}
}

void AFVAIController::OnUnPossess()
{
	if (CachedCharacter)
	{
		UE_LOG(LogTemp, Log, TEXT("AFVAIController: Unpossessed %s"), *CachedCharacter->GetName());
	}

	CachedCharacter = nullptr;

	Super::OnUnPossess();
}

AFVCharacter* AFVAIController::GetFVCharacter() const
{
	return CachedCharacter;
}

void AFVAIController::OnRegainedControlFromPlayer_Implementation()
{
	// Base implementation - override in Blueprint or C++ subclass
	// Could resume patrol, return to idle state, etc.
	UE_LOG(LogTemp, Log, TEXT("AFVAIController: Regained control of %s from player"), 
		CachedCharacter ? *CachedCharacter->GetName() : TEXT("Unknown"));
}