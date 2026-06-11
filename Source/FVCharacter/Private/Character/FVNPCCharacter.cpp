#include "Character/FVNPCCharacter.h"
#include "Interaction/FVInteractableComponent.h"

AFVNPCCharacter::AFVNPCCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	InteractableComponent = CreateDefaultSubobject<UFVInteractableComponent>(TEXT("InteractableComponent"));

	// NPCs don't replicate movement — single-player only
	SetReplicatingMovement(false);
}

void AFVNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
}
