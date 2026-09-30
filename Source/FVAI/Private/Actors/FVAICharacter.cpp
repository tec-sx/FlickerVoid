// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FVAICharacter.h"

#include "FVAICharacterController.h"
#include "FVAIConfigData.h"
#include "FVCoreTags.h"
#include "FVStateTreeAIComponent.h"
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

	if (AIConfig)
	{
		OwnedTags.AppendTags(AIConfig->StartingTags);
	}
}

void AFVAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AFVAICharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	FTimerHandle DummyHandle;
	GetWorld()->GetTimerManager().SetTimer(
		DummyHandle,
		[ this, NewController ] ()
		{
			if (const AFVAICharacterController* AIController = Cast<AFVAICharacterController>(NewController))
			{
				UStateTree* Tree = StateTree ? StateTree.Get() : (AIConfig ? AIConfig->DefaultStateTree.Get() : nullptr);
				if (AIController->GetStateTreeAIComponent() && Tree)
				{
					AIController->GetStateTreeAIComponent()->StartStateTree(Tree);
				}
			}
		},
		0.2f,
		false
	);
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


