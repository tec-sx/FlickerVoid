#include "Character/FVNPCCharacter.h"
#include "Interactions/FVInteractionTargetComponent.h"

AFVNPCCharacter::AFVNPCCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	InteractableComponent = CreateDefaultSubobject<UFVInteractionTargetComponent>(TEXT("InteractableComponent"));

	// NPCs don't replicate movement — single-player only
	SetReplicatingMovement(false);
}

void AFVNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
}
