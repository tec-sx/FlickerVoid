#include "Identity/FVIdentityComponent.h"

#include "GameFramework/Actor.h"
#include "Identity/FVIdentitySubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVIdentityComponent)

UFVIdentityComponent::UFVIdentityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVIdentityComponent* UFVIdentityComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVIdentityComponent>() : nullptr;
}

UFVCharacterDefinition* UFVIdentityComponent::GetActorDefinition(const AActor* Actor)
{
	const UFVIdentityComponent* Identity = Find(Actor);
	return Identity ? Identity->GetDefinition() : nullptr;
}

void UFVIdentityComponent::SetDefinition(UFVCharacterDefinition* NewDefinition)
{
	if (Definition == NewDefinition)
	{
		return;
	}

	Definition = NewDefinition;
	OnDefinitionChanged.Broadcast(Definition);
}

void UFVIdentityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UFVIdentitySubsystem* Subsystem = UFVIdentitySubsystem::Get(this))
	{
		Subsystem->Register(this);
	}
}

void UFVIdentityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVIdentitySubsystem* Subsystem = UFVIdentitySubsystem::Get(this))
	{
		Subsystem->Unregister(this);
	}

	Super::EndPlay(EndPlayReason);
}
