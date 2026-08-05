// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FVAICharacter.h"

#include "FVAICharacterController.h"
#include "FVCoreTags.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Flow/Components/FVFlowTriggerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAICharacter)

// Sets default values
AFVAICharacter::AFVAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
	AIControllerClass = AFVAICharacterController::StaticClass();
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

void AFVAICharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	TagContainer.AppendTags(OwnedTags);
}

void AFVAICharacter::AddGameplayTag(FGameplayTag Tag)
{
	OwnedTags.AddTag(Tag);
}

void AFVAICharacter::RemoveGameplayTag(FGameplayTag Tag)
{
	OwnedTags.RemoveTag(Tag);
}


