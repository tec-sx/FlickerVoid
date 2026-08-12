#include "Interactions/FVInteractionZoneComponent.h"

#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "GameFramework/Pawn.h"

UFVInteractionZoneComponent::UFVInteractionZoneComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UFVInteractionZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UFVInteractionZoneComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UFVInteractionZoneComponent::HandleEndOverlap);
}

UFVInteractionTargetComponent* UFVInteractionZoneComponent::GetOwnerTarget() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UFVInteractionTargetComponent>() : nullptr;
}

void UFVInteractionZoneComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	UFVInteractionInstigatorComponent* Instigator = Pawn->FindComponentByClass<UFVInteractionInstigatorComponent>();
	UFVInteractionTargetComponent* Target = GetOwnerTarget();
	if (Instigator && Target)
	{
		Instigator->RegisterCandidate(Target);
	}
}

void UFVInteractionZoneComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	UFVInteractionInstigatorComponent* Instigator = Pawn->FindComponentByClass<UFVInteractionInstigatorComponent>();
	UFVInteractionTargetComponent* Target = GetOwnerTarget();
	if (Instigator && Target)
	{
		Instigator->UnregisterCandidate(Target);
	}
}
