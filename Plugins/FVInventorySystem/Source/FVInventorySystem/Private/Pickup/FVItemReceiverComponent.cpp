#include "Pickup/FVItemReceiverComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemReceiverComponent)

UFVItemReceiverComponent* UFVItemReceiverComponent::FindReceiver(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UFVItemReceiverComponent* Receiver = Actor->FindComponentByClass<UFVItemReceiverComponent>())
	{
		return Receiver;
	}

	const APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn)
	{
		return nullptr;
	}

	if (const AController* Controller = Pawn->GetController())
	{
		if (UFVItemReceiverComponent* Receiver = Controller->FindComponentByClass<UFVItemReceiverComponent>())
		{
			return Receiver;
		}
	}

	if (const APlayerState* PlayerState = Pawn->GetPlayerState())
	{
		if (UFVItemReceiverComponent* Receiver = PlayerState->FindComponentByClass<UFVItemReceiverComponent>())
		{
			return Receiver;
		}
	}

	return nullptr;
}

int32 UFVItemReceiverComponent::ReceiveItem(UFVItemDefinition* Item, const int32 Quantity, AActor* Source)
{
	if (!Item || Quantity <= 0 || !bAcceptingItems)
	{
		return 0;
	}

	TakenQuantity = 0;
	OnItemReceived.Broadcast(Item, Quantity, Source);

	// Nothing reported back means the listener takes everything, e.g. a quest trigger.
	return OnItemReceived.IsBound() ? FMath::Min(TakenQuantity > 0 ? TakenQuantity : Quantity, Quantity) : 0;
}
