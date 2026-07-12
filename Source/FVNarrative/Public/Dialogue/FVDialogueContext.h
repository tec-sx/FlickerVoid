#pragma once
 
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVDialogueContext.generated.h"

UENUM(BlueprintType)
enum class EFVFactionStanding : uint8
{
	Hostile     = 0,
	Unfriendly  = 1,
	Neutral     = 2,
	Friendly    = 3,
	Allied      = 4
};

// ─────────────────────────────────────────────────────────────────────────────
// FDialogueContext
//
// Immutable snapshot built once per signal, before any condition evaluation.
// All evaluators and State Tree tasks read from this — never from live actors —
// so evaluation is deterministic within one signal dispatch.
//
// Built by UDialogueContextBuilder; owned by UDialogueSession.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FFVDialogueContext
{
	GENERATED_BODY()
	
	// ── Identity ──────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Speaker;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Listener;

	UPROPERTY(BlueprintReadOnly, meta=(Categories="Dialogue.Signal"))
	FGameplayTag Signal;
	
	UPROPERTY(BlueprintReadOnly, meta=(Categories="Dialogue.Speaker"))
	FGameplayTag SpeakerID;

	// ── Tags ──────────────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer WorldStateTags;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer InstigatorTags;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer TargetTags;

	// ── Numeric Values ────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly)
	int32 RelationshipValue = 0;
	
	UPROPERTY(BlueprintReadOnly)
	float NotorietyValue = 0.f;
	
	// ── Faction Standings ─────────────────────────────────────────────────────
	
    UPROPERTY(BlueprintReadOnly)
    TMap<FGameplayTag, EFVFactionStanding> FactionStandings;
 
    // ── Quest / Narrative ─────────────────────────────────────────────────────
	
    UPROPERTY(BlueprintReadOnly, meta=(Categories="Quest"))
    FGameplayTagContainer ActiveQuestTags;
 
    // ── Session History ───────────────────────────────────────────────────────
	
    UPROPERTY(BlueprintReadOnly)
    TSet<FName> ConsumedLineIDs;
	
    UPROPERTY(BlueprintReadOnly)
    TSet<FName> SessionPlayedLineIDs;
	
    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;
 
    // ── Helpers ───────────────────────────────────────────────────────────────
 
    bool IsValid() const
    {
        return Speaker.IsValid() && Listener.IsValid() && Signal.IsValid();
    }
 
    EFVFactionStanding GetFactionStanding(const FGameplayTag& FactionTag) const
    {
        if (const EFVFactionStanding* Standing = FactionStandings.Find(FactionTag))
        {
            return *Standing;
        }
        return EFVFactionStanding::Neutral;
    }
 
    bool HasConsumed(const FName& LineID) const
    {
        return ConsumedLineIDs.Contains(LineID) || SessionPlayedLineIDs.Contains(LineID);
    }
};
