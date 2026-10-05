#include "FVDialogueParticipantComponent.h"

#include "FVDialogueSubsystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueParticipantComponent)

UFVDialogueParticipantComponent::UFVDialogueParticipantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVDialogueParticipantComponent* UFVDialogueParticipantComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVDialogueParticipantComponent>() : nullptr;
}

bool UFVDialogueParticipantComponent::StartDialogue(AActor* Instigator)
{
	UFVDialogueSubsystem* Subsystem = UFVDialogueSubsystem::Get(this);
	return Subsystem && Dialogue && Subsystem->StartConversation(Dialogue, Instigator, GetOwner());
}
