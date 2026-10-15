#include "Items/FVFlowNode_GiveItem.h"

#include "Engine/World.h"
#include "FlowTags.h"
#include "FVFlowContext.h"
#include "FVGameplayTags.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemEventPayload.h"
#include "Items/FVItemLibrary.h"
#include "Items/FVItemMessages.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_GiveItem)

namespace FVGiveItemPins
{
	const FName Success = TEXT("Success");
	const FName Failed = TEXT("Failed");
}

UFVFlowNode_GiveItem::UFVFlowNode_GiveItem()
{
#if WITH_EDITOR
	Category = TEXT("FlickerVoid");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FVGiveItemPins::Success));
	OutputPins.Add(FFlowPin(FVGiveItemPins::Failed));
}

void UFVFlowNode_GiveItem::ExecuteInput(const FName& PinName)
{
	if (bWaiting)
	{
		return;
	}

	const FFVConditionContext FlowContext = FVFlow::MakeContext(*this);
	AActor* Player = FlowContext.Instigator;
	AActor* FlowOwner = FlowContext.Target;
	const bool bPlayerGives = Direction == EFVGiveDirection::PlayerGives;
	AActor* GiverActor = bPlayerGives ? Player : FlowOwner;
	AActor* ReceiverActor = bPlayerGives ? FlowOwner : Player;
	const int32 GivenQuantity = Item ? Quantity : 0;

	if (!Player || !FlowOwner || !UFVItemLibrary::CanGiveItem(GiverActor, ReceiverActor, Item, GivenQuantity, Money))
	{
		TriggerOutput(FVGiveItemPins::Failed, true);
		return;
	}

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	Giver = GiverActor;
	Receiver = ReceiverActor;
	bWaiting = true;
	// Check: Lyra GameplayMessageRouter API (member function RegisterListener, FGameplayMessageListenerHandle::Unregister).
	ListenerHandle = MessageSystem.RegisterListener(FVGameplayTags::Message_Item_GiveFinished, this, &UFVFlowNode_GiveItem::HandleGiveFinished);

	UFVItemEventPayload* Payload = NewObject<UFVItemEventPayload>(Player);
	Payload->Item = Item;
	Payload->OtherActor = FlowOwner;
	Payload->Quantity = GivenQuantity;
	Payload->Money = Money;
	Payload->bPlayerGives = bPlayerGives;
	Payload->bAsGift = bAsGift && bPlayerGives;
	const bool bHandled = UFVItemLibrary::SendItemPayload(Player, FVGameplayTags::Event_Item_Give, Payload);

	// A give ability may already have finished, and with it this node.
	if (!bWaiting || bHandled)
	{
		return;
	}

	// No give ability: give at once, without animation.
	FFVItemGiveMessage Message;
	Message.Giver = GiverActor;
	Message.Receiver = ReceiverActor;
	Message.Item = Item;
	Message.Quantity = GivenQuantity;
	Message.Money = Money;
	Message.bSucceeded = UFVItemLibrary::GiveItem(GiverActor, ReceiverActor, Item, GivenQuantity, Money, Payload->bAsGift);
	MessageSystem.BroadcastMessage(FVGameplayTags::Message_Item_GiveFinished, Message);
}

void UFVFlowNode_GiveItem::HandleGiveFinished(FGameplayTag Channel, const FFVItemGiveMessage& Message)
{
	if (!bWaiting || Message.Giver.Get() != Giver.Get() || Message.Receiver.Get() != Receiver.Get())
	{
		return;
	}

	bWaiting = false;
	TriggerOutput(Message.bSucceeded ? FVGiveItemPins::Success : FVGiveItemPins::Failed, true);
}

void UFVFlowNode_GiveItem::Cleanup()
{
	ListenerHandle.Unregister();
	ListenerHandle = FGameplayMessageListenerHandle();
	Giver.Reset();
	Receiver.Reset();
	bWaiting = false;

	Super::Cleanup();
}

#if WITH_EDITOR
FString UFVFlowNode_GiveItem::GetNodeDescription() const
{
	TArray<FString> Parts;
	if (Item)
	{
		const FString ItemName = Item->Display.Name.IsEmpty() ? Item->GetName() : Item->Display.Name.ToString();
		Parts.Add(FString::Printf(TEXT("%d x %s"), Quantity, *ItemName));
	}
	if (Money > 0)
	{
		Parts.Add(FString::Printf(TEXT("%d money"), Money));
	}
	if (Parts.IsEmpty())
	{
		return TEXT("Nothing to give");
	}

	const bool bPlayerGives = Direction == EFVGiveDirection::PlayerGives;
	FString Description = FString::Printf(TEXT("Player %s %s"), bPlayerGives ? TEXT("gives") : TEXT("receives"), *FString::Join(Parts, TEXT(" and ")));
	if (bAsGift && bPlayerGives)
	{
		Description += TEXT(" as a gift");
	}
	return Description;
}

EDataValidationResult UFVFlowNode_GiveItem::ValidateNode()
{
	if (!Item && Money <= 0)
	{
		ValidationLog.Error<UFlowNode>(TEXT("Nothing to give: set an Item or Money"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif
