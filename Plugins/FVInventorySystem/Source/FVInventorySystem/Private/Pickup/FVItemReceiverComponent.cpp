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
	FFVItemStack Stack;
	Stack.Item = Item;
	Stack.Quantity = Quantity;
	return ReceiveStack(Stack, Source);
}

int32 UFVItemReceiverComponent::ReceiveStack(const FFVItemStack& Stack, AActor* Source)
{
	if (!Stack.Item || Stack.Quantity <= 0 || !bAcceptingItems)
	{
		return 0;
	}

	TakenQuantity = 0;
	bTakenReported = false;

	const bool bPlain = Stack.InstanceData.IsEmpty();
	OnStackReceived.Broadcast(Stack, Source);
	if (bPlain)
	{
		OnItemReceived.Broadcast(Stack.Item, Stack.Quantity, Source);
	}

	if (bTakenReported)
	{
		return FMath::Min(TakenQuantity, Stack.Quantity);
	}

	// Nothing reported back means the listener takes everything, e.g. a quest trigger.
	return OnStackReceived.IsBound() || (bPlain && OnItemReceived.IsBound()) ? Stack.Quantity : 0;
}
