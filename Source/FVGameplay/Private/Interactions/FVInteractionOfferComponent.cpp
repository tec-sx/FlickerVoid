#include "Interactions/FVInteractionOfferComponent.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "FVGameplayTags.h"
#include "Interactions/FVInteractionAction.h"
#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionMessageTypes.h"
#include "Interactions/FVInteractionTargetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionOfferComponent)

namespace
{
	constexpr int32 FocusOfferPriority = 0;

	EFVInteractionCancelReason ToCancelReason(EFVInteractionOfferOutcome Outcome)
	{
		switch (Outcome)
		{
		case EFVInteractionOfferOutcome::Expired:
			return EFVInteractionCancelReason::OfferExpired;
		case EFVInteractionOfferOutcome::Superseded:
			return EFVInteractionCancelReason::HigherPriorityOffer;
		case EFVInteractionOfferOutcome::Withdrawn:
			return EFVInteractionCancelReason::WalkedAway;
		default:
			return EFVInteractionCancelReason::None;
		}
	}
}

UFVInteractionOfferComponent::UFVInteractionOfferComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFVInteractionOfferComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		if (UFVInteractionInstigatorComponent* Instigator =
			OwnerActor->FindComponentByClass<UFVInteractionInstigatorComponent>())
		{
			Instigator->OnFocusChanged.AddDynamic(this, &UFVInteractionOfferComponent::HandleFocusChanged);
		}
	}
}

void UFVInteractionOfferComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (UFVInteractionInstigatorComponent* Instigator =
			OwnerActor->FindComponentByClass<UFVInteractionInstigatorComponent>())
		{
			Instigator->OnFocusChanged.RemoveDynamic(this, &UFVInteractionOfferComponent::HandleFocusChanged);
		}
	}

	Offers.Reset();
	ActiveOffer = FFVInteractionOffer();

	Super::EndPlay(EndPlayReason);
}

void UFVInteractionOfferComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RefreshEngagement();
	RefreshOffers(DeltaTime);
	RecomputeActiveOffer();
}

//~=============================================================================
// Focus source
//~=============================================================================

void UFVInteractionOfferComponent::HandleFocusChanged(UFVInteractionTargetComponent* Target)
{
	const int32 ExistingIndex = Offers.IndexOfByPredicate(
		[](const FFVInteractionOffer& Offer) { return Offer.Source == EFVInteractionOfferSource::Focus; });

	if (!Target)
	{
		if (ExistingIndex != INDEX_NONE)
		{
			FinishOffer(Offers[ExistingIndex].OfferId, EFVInteractionOfferOutcome::Withdrawn);
		}
		return;
	}

	if (ExistingIndex != INDEX_NONE)
	{
		Offers[ExistingIndex].Target = Target;
		ResolveOfferInto(Offers[ExistingIndex]);
		return;
	}

	FFVInteractionOffer Offer;
	Offer.OfferId = NextOfferId++;
	Offer.Source = EFVInteractionOfferSource::Focus;
	Offer.Target = Target;
	Offer.Priority = FocusOfferPriority;
	ResolveOfferInto(Offer);

	Offers.Add(MoveTemp(Offer));
}

//~=============================================================================
// Scripted offers
//~=============================================================================

int32 UFVInteractionOfferComponent::PushScriptedOffer(
	UFVInteractionTargetComponent* Target,
	int32 Priority,
	bool bExclusive,
	float Timeout,
	EFVInteractionSlot DefaultSlot)
{
	if (!Target)
	{
		return INDEX_NONE;
	}

	FFVInteractionOffer Offer;
	Offer.OfferId = NextOfferId++;
	Offer.Source = EFVInteractionOfferSource::Scripted;
	Offer.Target = Target;
	Offer.Priority = Priority;
	Offer.bExclusive = bExclusive;
	Offer.Timeout = FMath::Max(0.f, Timeout);
	Offer.TimeRemaining = Offer.Timeout;
	Offer.DefaultSlot = DefaultSlot;
	ResolveOfferInto(Offer);

	const int32 OfferId = Offer.OfferId;
	Offers.Add(MoveTemp(Offer));

	RecomputeActiveOffer();

	return OfferId;
}

void UFVInteractionOfferComponent::WithdrawOffer(int32 OfferId)
{
	FinishOffer(OfferId, EFVInteractionOfferOutcome::Withdrawn);
}

void UFVInteractionOfferComponent::NotifyActiveOfferTaken()
{
	if (ActiveOffer.IsValidOffer())
	{
		UFVInteractionTargetComponent* Target = ActiveOffer.Target;
		FinishOffer(ActiveOffer.OfferId, EFVInteractionOfferOutcome::Taken);
		SetEngagedTarget(Target && Target->IsInteractionInProgress() ? Target : nullptr);
	}
}

void UFVInteractionOfferComponent::AbortEngagedInteraction(EFVInteractionCancelReason Reason)
{
	if (UFVInteractionTargetComponent* Target = EngagedTarget.Get())
	{
		Target->CancelActiveInteraction(Reason);
	}

	SetEngagedTarget(nullptr);
}

void UFVInteractionOfferComponent::SetEngagedTarget(UFVInteractionTargetComponent* Target)
{
	if (EngagedTarget.Get() == Target)
	{
		return;
	}

	EngagedTarget = Target;
	OnEngagementChanged.Broadcast(Target);
}

void UFVInteractionOfferComponent::RefreshEngagement()
{
	// The target's tree owns execution; engagement ends when that tree stops.
	UFVInteractionTargetComponent* Target = EngagedTarget.Get();
	if (EngagedTarget.IsValid() && (!Target || !Target->IsInteractionInProgress()))
	{
		SetEngagedTarget(nullptr);
	}
}

//~=============================================================================
// Resolution
//~=============================================================================

FFVResolvedInteraction UFVInteractionOfferComponent::GetActiveSlot(EFVInteractionSlot Slot) const
{
	if (!ActiveOffer.IsValidOffer() || Slot >= EFVInteractionSlot::MAX)
	{
		return FFVResolvedInteraction();
	}

	return ActiveOffer.Resolved.GetSlot(Slot);
}

void UFVInteractionOfferComponent::RefreshOffers(float DeltaTime)
{
	TArray<int32, TInlineAllocator<4>> ExpiredIds;
	TArray<int32, TInlineAllocator<4>> StaleIds;

	for (FFVInteractionOffer& Offer : Offers)
	{
		if (!Offer.Target)
		{
			StaleIds.Add(Offer.OfferId);
			continue;
		}

		// Re-resolve every tick so requirement changes are reflected live.
		ResolveOfferInto(Offer);

		if (Offer.IsTimed())
		{
			Offer.TimeRemaining = FMath::Max(0.f, Offer.TimeRemaining - DeltaTime);

			if (Offer.TimeRemaining <= 0.f)
			{
				ExpiredIds.Add(Offer.OfferId);
			}
		}
	}

	for (const int32 OfferId : StaleIds)
	{
		FinishOffer(OfferId, EFVInteractionOfferOutcome::Withdrawn);
	}

	for (const int32 OfferId : ExpiredIds)
	{
		const int32 Index = Offers.IndexOfByPredicate(
			[OfferId](const FFVInteractionOffer& Offer) { return Offer.OfferId == OfferId; });

		if (Index == INDEX_NONE)
		{
			continue;
		}

		const FFVInteractionOffer& Offer = Offers[Index];

		if (Offer.DefaultSlot < EFVInteractionSlot::MAX && Offer.Target)
		{
			const FFVResolvedInteraction& Fallback = Offer.Resolved.GetSlot(Offer.DefaultSlot);

			if (Fallback.IsBound() && Fallback.Info.bAvailable && !Offer.Target->IsInteractionInProgress())
			{
				Offer.Target->RunAction(GetOwner(), Fallback.Action);
			}
		}

		FinishOffer(OfferId, EFVInteractionOfferOutcome::Expired);
	}
}

void UFVInteractionOfferComponent::RecomputeActiveOffer()
{
	const FFVInteractionOffer* Winner = nullptr;

	for (const FFVInteractionOffer& Offer : Offers)
	{
		if (!Offer.Resolved.HasAnyBinding())
		{
			continue;
		}

		if (!Winner || Offer.Priority > Winner->Priority)
		{
			Winner = &Offer;
		}
	}

	// An exclusive winner suppresses everything else, which is already the
	// behaviour of picking a single winner; the flag matters to callers that
	// inspect the active offer to decide whether to suppress other HUD elements.
	const FFVInteractionOffer NewOffer = Winner ? *Winner : FFVInteractionOffer();

	const bool bChanged =
		NewOffer.OfferId != ActiveOffer.OfferId ||
		NewOffer.Target != ActiveOffer.Target;

	ActiveOffer = NewOffer;

	if (bChanged)
	{
		OnActiveOfferChanged.Broadcast(ActiveOffer);
	}

	// Broadcast every recompute so live availability and countdown changes reach
	// the UI; the router dirty-checks and drops redundant updates.
	BroadcastOfferMessage();
}

void UFVInteractionOfferComponent::BroadcastOfferMessage() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FFVInteractionOfferChangedMessage Message;
	Message.Offer = ActiveOffer;
	Message.bHasOffer = ActiveOffer.IsValidOffer();

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		Message.OwningController = Cast<APlayerController>(OwnerPawn->GetController());
	}

	UGameplayMessageSubsystem::Get(World).BroadcastMessage(
		FVGameplayTags::Interaction_OfferChanged,
		Message);
}

void UFVInteractionOfferComponent::ResolveOfferInto(FFVInteractionOffer& Offer) const
{
	Offer.Resolved = UFVInteractionResolver::ResolveInteractions(Offer.Target, GetInstigatorTags());
}

void UFVInteractionOfferComponent::FinishOffer(int32 OfferId, EFVInteractionOfferOutcome Outcome)
{
	const int32 Index = Offers.IndexOfByPredicate(
		[OfferId](const FFVInteractionOffer& Offer) { return Offer.OfferId == OfferId; });

	if (Index == INDEX_NONE)
	{
		return;
	}

	const FFVInteractionOffer Finished = Offers[Index];
	Offers.RemoveAt(Index);

	// Withdrawal and expiry must interrupt a running interaction using the same
	// path as walking away, so tasks see a meaningful reason in ExitState.
	if (Outcome != EFVInteractionOfferOutcome::Taken && Finished.Target)
	{
		if (Finished.Target->IsInteractionInProgress())
		{
			Finished.Target->CancelActiveInteraction(ToCancelReason(Outcome));
		}
	}

	OnOfferResolved.Broadcast(Finished, Outcome);

	RecomputeActiveOffer();
}

FGameplayTagContainer UFVInteractionOfferComponent::GetInstigatorTags() const
{
	FGameplayTagContainer Tags;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return Tags;
	}

	if (const IGameplayTagAssetInterface* TagOwner = Cast<IGameplayTagAssetInterface>(OwnerActor))
	{
		TagOwner->GetOwnedGameplayTags(Tags);
	}
	else if (const IAbilitySystemInterface* AbilityOwner = Cast<IAbilitySystemInterface>(OwnerActor))
	{
		if (const UAbilitySystemComponent* ASC = AbilityOwner->GetAbilitySystemComponent())
		{
			ASC->GetOwnedGameplayTags(Tags);
		}
	}

	return Tags;
}
