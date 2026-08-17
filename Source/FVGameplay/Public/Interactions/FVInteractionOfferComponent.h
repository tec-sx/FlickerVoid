#pragma once

#include "CoreMinimal.h"
#include "FVInteractionTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "UI/FVInteractionInfo.h"
#include "Interactions/FVInteractionConfig.h"
#include "FVInteractionOfferComponent.generated.h"

class UFVInteractionTargetComponent;

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVResolvedInteraction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVInteractionConfig Config;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVInteractionInfo Info;

	bool IsBound() const { return Config.IsValid(); }
};


USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVResolvedInteractionSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TArray<FFVResolvedInteraction> Slots;

	FFVResolvedInteractionSet()
	{
		Slots.SetNum(static_cast<int32>(EFVInteractionSlot::MAX));
	}

	const FFVResolvedInteraction& GetSlot(EFVInteractionSlot Slot) const
	{
		return Slots[static_cast<int32>(Slot)];
	}

	bool HasAnyBinding() const
	{
		return Slots.ContainsByPredicate([](const FFVResolvedInteraction& Entry) { return Entry.IsBound(); });
	}
};

// A focus offer is the ordinary "look at a door, see Open" case.A scripted offer
// is a cinematic case, pushed by a narrative.
UENUM(BlueprintType)
enum class EFVInteractionOfferSource : uint8
{
	Focus,
	Scripted,
};

UENUM(BlueprintType)
enum class EFVInteractionOfferOutcome : uint8
{
	Taken,
	Expired,
	Withdrawn,
	Superseded,
};

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

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	int32 Priority = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	bool bExclusive = false;

	// Seconds before the offer auto-resolves. <= 0 means it never expires.
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	float Timeout = 0.f;

	// Slot auto-taken when the timeout elapses. MAX means "just expire".
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	EFVInteractionSlot DefaultSlot = EFVInteractionSlot::MAX;

	// Seconds remaining; only meaningful when Timeout > 0.
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Offer")
	float TimeRemaining = 0.f;

	bool IsValidOffer() const { return OfferId != INDEX_NONE; }
	bool IsTimed() const { return Timeout > 0.f; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveOfferChanged, const FFVInteractionOffer&, Offer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOfferResolved, const FFVInteractionOffer&, Offer, EFVInteractionOfferOutcome, Outcome);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionEngagementChanged, UFVInteractionTargetComponent*, Target);


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

	// Queries

	UFUNCTION(BlueprintPure, Category = "Interaction|Offer")
	const FFVInteractionOffer& GetActiveOffer() const { return ActiveOffer; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Offer")
	bool HasActiveOffer() const { return ActiveOffer.IsValidOffer(); }

	UFUNCTION(BlueprintPure, Category = "Interaction|Offer")
	FFVResolvedInteraction GetActiveSlot(EFVInteractionSlot Slot) const;

	// Scripted offers

	UFUNCTION(BlueprintCallable, Category = "Interaction|Offer")
	int32 PushScriptedOffer(
		UFVInteractionTargetComponent* Target,
		int32 Priority,
		bool bExclusive,
		float Timeout,
		EFVInteractionSlot DefaultSlot);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Offer")
	void WithdrawOffer(int32 OfferId);

	// Engagement

	UFUNCTION(BlueprintPure, Category = "Interaction|Engagement")
	UFVInteractionTargetComponent* GetEngagedTarget() const { return EngagedTarget.Get(); }
	
	UFUNCTION(BlueprintPure, Category = "Interaction|Engagement")
	AActor* GetEngagedActor() const;

	UFUNCTION(BlueprintPure, Category = "Interaction|Engagement")
	bool IsInteracting() const { return EngagedTarget.IsValid(); }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Engagement")
	EFVInteractionResult BeginEngagement(EFVInteractionSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Engagement")
	void EndEngagement();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Engagement")
	void AbortEngagedInteraction(EFVInteractionCancelReason Reason);

	// Events

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Offer")
	FOnActiveOfferChanged OnActiveOfferChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Offer")
	FOnOfferResolved OnOfferResolved;

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
	void FinishOffer(int32 OfferId, EFVInteractionOfferOutcome Outcome);
	void NotifyActiveOfferTaken();

	UPROPERTY(Transient)
	TArray<FFVInteractionOffer> Offers;

	UPROPERTY(Transient)
	FFVInteractionOffer ActiveOffer;

	int32 NextOfferId = 0;

	TWeakObjectPtr<UFVInteractionTargetComponent> EngagedTarget;

	static FFVResolvedInteractionSet ResolveInteractions(
		UFVInteractionTargetComponent* Target,
		AActor* Instigator);
};
