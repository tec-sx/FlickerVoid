#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactions/FVInteractionResolver.h"
#include "FVInteractionOfferComponent.generated.h"

class UFVInteractionAction;
class UFVInteractionTargetComponent;

/**
 * Where an offer came from. Focus offers are regenerated continuously from the
 * instigator component; scripted offers are pushed explicitly and outrank focus.
 */
UENUM(BlueprintType)
enum class EFVInteractionOfferSource : uint8
{
	Focus,
	Scripted,
};

/** How an offer ended, reported to listeners when it is withdrawn. */
UENUM(BlueprintType)
enum class EFVInteractionOfferOutcome : uint8
{
	Taken,
	Expired,
	Withdrawn,
	Superseded,
};

/**
 * A prioritized bundle of slot bindings presented to the player at one moment.
 *
 * A focus offer is the ordinary "look at a door, see Open" case. A scripted offer
 * is the cinematic case: pushed by a narrative beat, higher priority, optionally
 * exclusive (suppresses focus offers entirely) and optionally timed with a
 * default outcome when the player fails to act.
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionOffer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	int32 OfferId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	EFVInteractionOfferSource Source = EFVInteractionOfferSource::Focus;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	FFVResolvedInteractionSet Resolved;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	TObjectPtr<UFVInteractionTargetComponent> Target = nullptr;

	/** Higher wins. Focus offers use 0 by convention. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	int32 Priority = 0;

	/** When true, lower-priority offers are suppressed entirely rather than stacked. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	bool bExclusive = false;

	/** Seconds before the offer auto-resolves. <= 0 means it never expires. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	float Timeout = 0.f;

	/** Slot auto-taken when the timeout elapses. MAX means "just expire". */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	EFVInteractionSlot DefaultSlot = EFVInteractionSlot::MAX;

	/** Seconds remaining; only meaningful when Timeout > 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	float TimeRemaining = 0.f;

	bool IsValidOffer() const { return OfferId != INDEX_NONE; }
	bool IsTimed() const { return Timeout > 0.f; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnActiveOfferChanged,
	const FFVInteractionOffer&, Offer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnOfferResolved,
	const FFVInteractionOffer&, Offer,
	EFVInteractionOfferOutcome, Outcome);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnInteractionEngagementChanged,
	UFVInteractionTargetComponent*, Target);

/**
 * Player-side owner of everything currently being offered to the player.
 *
 * Focus is just one source among several: it is refreshed from the instigator
 * component's focus changes and re-resolved each tick so requirement changes are
 * reflected live. Scripted offers can be pushed on top with higher priority.
 * The single highest-priority offer is what the UI shows and what input dispatches to.
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionOfferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionOfferComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	//~=========================================================================
	// Queries
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction|Offer")
	const FFVInteractionOffer& GetActiveOffer() const { return ActiveOffer; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Offer")
	bool HasActiveOffer() const { return ActiveOffer.IsValidOffer(); }

	/** Resolves a slot against the active offer. Returns an unbound entry if nothing occupies it. */
	UFUNCTION(BlueprintPure, Category = "Interaction|Offer")
	FFVResolvedInteraction GetActiveSlot(EFVInteractionSlot Slot) const;

	//~=========================================================================
	// Scripted offers
	//~=========================================================================

	/** Pushes a scripted offer. Returns its id, or INDEX_NONE if it could not be built. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Offer")
	int32 PushScriptedOffer(
		UFVInteractionTargetComponent* Target,
		int32 Priority,
		bool bExclusive,
		float Timeout,
		EFVInteractionSlot DefaultSlot);

	/** Withdraws a previously pushed offer by id. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Offer")
	void WithdrawOffer(int32 OfferId);

	/** Marks the active offer as taken (called after an interaction successfully starts). */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Offer")
	void NotifyActiveOfferTaken();

	//~=========================================================================
	// Engagement — handshake with the player State Tree
	//~=========================================================================

	/** The target whose tree the player is currently committed to, if any. */
	UFUNCTION(BlueprintPure, Category = "Interaction|Engagement")
	UFVInteractionTargetComponent* GetEngagedTarget() const { return EngagedTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction|Engagement")
	bool IsInteracting() const { return EngagedTarget.IsValid(); }

	/** Aborts the engaged interaction; the target tree exits with this reason. */
	UFUNCTION(BlueprintCallable, Category = "Interaction|Engagement")
	void AbortEngagedInteraction(EFVInteractionCancelReason Reason);

	//~=========================================================================
	// Events
	//~=========================================================================

	/** Fires whenever the winning offer changes identity or contents. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Offer")
	FOnActiveOfferChanged OnActiveOfferChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Offer")
	FOnOfferResolved OnOfferResolved;

	/** Fires when the player commits to, or disengages from, a target's interaction. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Engagement")
	FOnInteractionEngagementChanged OnEngagementChanged;

private:
	UFUNCTION()
	void HandleFocusChanged(UFVInteractionTargetComponent* Target);

	void RefreshOffers(float DeltaTime);
	void RefreshEngagement();
	void SetEngagedTarget(UFVInteractionTargetComponent* Target);
	void RecomputeActiveOffer();
	void BroadcastOfferMessage() const;
	void ResolveOfferInto(FFVInteractionOffer& Offer) const;
	void FinishOffer(int32 OfferId, EFVInteractionOfferOutcome Outcome);
	FGameplayTagContainer GetInstigatorTags() const;

	UPROPERTY(Transient)
	TArray<FFVInteractionOffer> Offers;

	UPROPERTY(Transient)
	FFVInteractionOffer ActiveOffer;

	int32 NextOfferId = 0;

	TWeakObjectPtr<UFVInteractionTargetComponent> EngagedTarget;
};
