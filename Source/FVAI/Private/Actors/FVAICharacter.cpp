// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FVAICharacter.h"

#include "FVAICharacterController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AFVAICharacter::AFVAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
	AIControllerClass = AFVAICharacterController::StaticClass();
	
	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Sphere"));
	InteractionZone->SetupAttachment(GetCapsuleComponent());
	
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float BoxExtentY = CapsuleRadius * 3.5;
	const float BoxExtentX = CapsuleRadius * 3.7;
	constexpr float InteractionBackOffset = 12.f;
	const FVector InteractionExtent = FVector(BoxExtentX, BoxExtentY,CapsuleHalfHeight);
	
	InteractionZone->SetBoxExtent(InteractionExtent);
	InteractionZone->SetWorldLocation(FVector(BoxExtentX - InteractionBackOffset, 0, 0));
}

void AFVAICharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFVAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AFVAICharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AFVAICharacter::UnPossessed()
{
	Super::UnPossessed();
}


