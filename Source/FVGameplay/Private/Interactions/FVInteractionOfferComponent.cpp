#include "Interactions/FVInteractionOfferComponent.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/FVAbilitySystemComponent.h"
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
		FinishOffer(ActiveOffer.OfferId, EFVInteractionOfferOutcome::Taken);
	}
}

void UFVInteractionOfferComponent::BeginEngagement(
	UFVInteractionTargetComponent* Target,
	FGameplayAbilitySpecHandle AbilityHandle)
{
	EngagedAbilityHandle = AbilityHandle;

	// Engagement is bounded by the dispatched ability, so no interaction ability
	// has to remember to release it.
	if (UAbilitySystemComponent* ASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(
			this, &UFVInteractionOfferComponent::HandleAbilityEnded);
	}

	SetEngagedTarget(Target);
}

void UFVInteractionOfferComponent::EndEngagement()
{
	if (AbilityEndedHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
		}

		AbilityEndedHandle.Reset();
	}

	EngagedAbilityHandle = FGameplayAbilitySpecHandle();
	SetEngagedTarget(nullptr);
}

void UFVInteractionOfferComponent::HandleAbilityEnded(const FAbilityEndedData& EndedData)
{
	if (EndedData.AbilitySpecHandle == EngagedAbilityHandle)
	{
		EndEngagement();
	}
}

namespace
{
	FGameplayTag ToCancelTag(EFVInteractionCancelReason Reason)
	{
		switch (Reason)
		{
			case EFVInteractionCancelReason::WalkedAway:          return FVGameplayTags::Interaction_Cancel_WalkedAway;
			case EFVInteractionCancelReason::HigherPriorityOffer: return FVGameplayTags::Interaction_Cancel_HigherPriorityOffer;
			case EFVInteractionCancelReason::OfferExpired:        return FVGameplayTags::Interaction_Cancel_OfferExpired;
			case EFVInteractionCancelReason::CombatStarted:       return FVGameplayTags::Interaction_Cancel_CombatStarted;
			case EFVInteractionCancelReason::Death:               return FVGameplayTags::Interaction_Cancel_Death;
			case EFVInteractionCancelReason::Scripted:            return FVGameplayTags::Interaction_Cancel_Scripted;
			default:                                              return FGameplayTag::EmptyTag;
		}
	}
}

void UFVInteractionOfferComponent::AbortEngagedInteraction(EFVInteractionCancelReason Reason)
{
	if (EngagedAbilityHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
		{
			// The reason is published as a loose tag for the duration of the cancel
			// so the dispatched ability can branch on it inside EndAbility.
			const FGameplayTag ReasonTag = ToCancelTag(Reason);

			if (ReasonTag.IsValid())
			{
				ASC->AddLooseGameplayTag(ReasonTag);
			}

			ASC->CancelAbilityHandle(EngagedAbilityHandle);

			if (ReasonTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(ReasonTag);
			}
		}
	}

	EndEngagement();
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
	// The dispatched ability owns engagement; only reap a target that went away.
	if (EngagedTarget.IsStale())
	{
		EndEngagement();
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

		UFVInteractionTargetComponent* FallbackTarget = Offer.Target;
		FGameplayTag FallbackAbilityTag;

		if (Offer.DefaultSlot < EFVInteractionSlot::MAX && FallbackTarget && !IsInteracting())
		{
			const FFVResolvedInteraction& Fallback = Offer.Resolved.GetSlot(Offer.DefaultSlot);

			if (Fallback.IsBound() && Fallback.Info.bAvailable)
			{
				FallbackAbilityTag = Fallback.Action->AbilityTag;
			}
		}

		FGameplayAbilitySpecHandle DispatchedHandle;

		if (FallbackAbilityTag.IsValid())
		{
			if (UFVAbilitySystemComponent* ASC = Cast<UFVAbilitySystemComponent>(
				UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner())))
			{
				DispatchedHandle = ASC->TryActivateAbilityByAssetTagAndGetHandle(FallbackAbilityTag);
			}
		}

		if (DispatchedHandle.IsValid())
		{
			FinishOffer(OfferId, EFVInteractionOfferOutcome::Taken);
			BeginEngagement(FallbackTarget, DispatchedHandle);
		}
		else
		{
			FinishOffer(OfferId, EFVInteractionOfferOutcome::Expired);
		}
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
	Offer.Resolved = UFVInteractionResolver::ResolveInteractions(Offer.Target, GetOwner());
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
	// path as walking away, so the dispatched ability sees a meaningful reason.
	if (Outcome != EFVInteractionOfferOutcome::Taken &&
		Finished.Target &&
		Finished.Target == EngagedTarget.Get())
	{
		AbortEngagedInteraction(ToCancelReason(Outcome));
	}

	OnOfferResolved.Broadcast(Finished, Outcome);

	RecomputeActiveOffer();
}
