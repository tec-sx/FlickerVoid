#include "Interfaces/FVItemReceiver.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemReceiver)

namespace
{
	UObject* FindReceiverOn(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (Actor->Implements<UFVItemReceiver>())
		{
			return Actor;
		}

		TArray<UActorComponent*> Receivers = Actor->GetComponentsByInterface(UFVItemReceiver::StaticClass());
		return Receivers.IsEmpty() ? nullptr : Receivers[0];
	}
}

UObject* IFVItemReceiver::FindReceiver(AActor* Actor)
{
	if (UObject* Receiver = FindReceiverOn(Actor))
	{
		return Receiver;
	}

	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (UObject* Receiver = FindReceiverOn(Pawn->GetController()))
		{
			return Receiver;
		}

		return FindReceiverOn(Pawn->GetPlayerState());
	}

	return nullptr;
}
