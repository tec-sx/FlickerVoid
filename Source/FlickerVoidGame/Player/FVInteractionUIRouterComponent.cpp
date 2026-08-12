#include "FVInteractionUIRouterComponent.h"

#include "FVGameplayTags.h"
#include "FVUITags.h"
#include "GameFramework/PlayerController.h"
#include "Interactions/FVInteractionMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionUIRouterComponent)

UFVInteractionUIRouterComponent::UFVInteractionUIRouterComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }

void UFVInteractionUIRouterComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	ListenerHandle = Router.RegisterListener(
		FVGameplayTags::Interaction_OfferChanged,
		this,
		&UFVInteractionUIRouterComponent::HandleOfferChanged);
}

void UFVInteractionUIRouterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem::Get(this).UnregisterListener(ListenerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInteractionUIRouterComponent::HandleOfferChanged(
	FGameplayTag Channel,
	const FFVInteractionOfferChangedMessage& Message)
{
	// Only route offers belonging to this controller.
	if (Message.OwningController != Cast<APlayerController>(GetOwner()))
	{
		return;
	}

	FFVUIInteractionPromptMessage Prompt;
	Prompt.bVisible = Message.bHasOffer;

	if (Message.bHasOffer)
	{
		const FFVInteractionOffer& Offer = Message.Offer;

		Prompt.bTimed = Offer.IsTimed();
		Prompt.TimeRemainingRatio = Offer.IsTimed()
			? FMath::Clamp(Offer.TimeRemaining / Offer.Timeout, 0.f, 1.f)
			: 0.f;

		const int32 SlotCount = static_cast<int32>(EFVInteractionSlot::MAX);
		Prompt.Slots.Reserve(SlotCount);

		for (int32 Index = 0; Index < SlotCount; ++Index)
		{
			const EFVInteractionSlot SlotId = static_cast<EFVInteractionSlot>(Index);
			const FFVResolvedInteraction& Entry = Offer.Resolved.GetSlot(SlotId);

			FFVUIInteractionSlot UISlot;
			UISlot.Slot = SlotId;
			UISlot.bOccupied = Entry.IsBound();

			if (Entry.IsBound())
			{
				UISlot.DisplayName = Entry.Info.DisplayName;
				UISlot.Icon = Entry.Info.Icon;
				UISlot.bEnabled = Entry.Info.bAvailable;
				UISlot.DisabledReason = Entry.Info.UnavailableReason;

				if (const FText* Hint = SlotInputHints.Find(SlotId))
				{
					UISlot.InputHint = *Hint;
				}
			}

			Prompt.Slots.Add(MoveTemp(UISlot));
		}
	}

	if (bHasBroadcast && ArePromptsEquivalent(LastPrompt, Prompt))
	{
		return;
	}

	LastPrompt = Prompt;
	bHasBroadcast = true;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		FVUITags::UI_Interaction_PromptChanged,
		Prompt);
}

bool UFVInteractionUIRouterComponent::ArePromptsEquivalent(
	const FFVUIInteractionPromptMessage& A,
	const FFVUIInteractionPromptMessage& B)
{
	if (A.bVisible != B.bVisible || A.bTimed != B.bTimed || A.Slots.Num() != B.Slots.Num())
	{
		return false;
	}

	// Countdown changes every tick, so compare it coarsely to avoid spamming the UI.
	if (A.bTimed && !FMath::IsNearlyEqual(A.TimeRemainingRatio, B.TimeRemainingRatio, 0.01f))
	{
		return false;
	}

	for (int32 Index = 0; Index < A.Slots.Num(); ++Index)
	{
		const FFVUIInteractionSlot& SlotA = A.Slots[Index];
		const FFVUIInteractionSlot& SlotB = B.Slots[Index];

		if (SlotA.bOccupied != SlotB.bOccupied ||
			SlotA.bEnabled != SlotB.bEnabled ||
			SlotA.Icon != SlotB.Icon ||
			!SlotA.DisplayName.EqualTo(SlotB.DisplayName) ||
			!SlotA.DisabledReason.EqualTo(SlotB.DisabledReason))
		{
			return false;
		}
	}

	return true;
}
