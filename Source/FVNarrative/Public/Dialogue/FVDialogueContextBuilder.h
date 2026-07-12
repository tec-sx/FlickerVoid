// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Dialogue/FVDialogueContext.h"
#include "FVDialogueContextBuilder.generated.h"

UCLASS(Blueprintable, BlueprintType)
class FLICKERVOIDNARRATIVE_API UFVDialogueContextBuilder : public UObject
{
	GENERATED_BODY()
	
public:

	/**
	 * Build a complete context snapshot.
	*/
    virtual FFVDialogueContext Build(
		const FGameplayTag& SpeakerID,
        const FGameplayTag& Signal,
        AActor* Speaker,
        AActor* Listener,
        const TSet<FName>& ConsumedLineIDs) const;
 
    // ─────────────────────────────────────────────────────────────────────────
    // Integration hooks — override in your project subclass
    //
    // These are separated out so you can override only the parts that differ
    // from the default implementations without reimplementing Build().
    // ─────────────────────────────────────────────────────────────────────────
	
    /**
     * Return current world state tags.
     * Default: reads from your WorldStateSubsystem (if present) or
     * derives Day/Night from world time.
     * Override to integrate your world state manager.
     */
    virtual FGameplayTagContainer BuildWorldStateTags(UWorld* World) const;
 
    /**
     * Return gameplay tags on the player/instigator.
     * Default: reads from GAS AbilitySystemComponent if present.
     * Override to use your own tag source.
     */
    virtual FGameplayTagContainer BuildInstigatorTags(AActor* InstigatorActor) const;
 
    /**
     * Return gameplay tags describing the NPC (personality, state, etc.).
     * Default: reads from GAS AbilitySystemComponent if present.
     * Override to use your own NPC tag system.
     */
    virtual FGameplayTagContainer BuildTargetTags(AActor* TargetActor) const;
 
    /**
     * Return the relationship value between instigator and target.
     * Default: 0 (no relationship system). Override to query your tracker.
     */
    virtual int32 ResolveRelationshipValue(AActor* Instigator, AActor* Target) const;
 
    /**
     * Return the player's notoriety value [0..100].
     * Default: 0. Override to query your stats system.
     */
    virtual float ResolveNotorietyValue(AActor* Instigator) const;
 
    /**
     * Return faction standings for the player.
     * Default: empty map (all Neutral). Override to query your faction system.
     */
    virtual TMap<FGameplayTag, EFVFactionStanding> BuildFactionStandings(AActor* Instigator) const;
 
    /**
     * Return all currently active/completed quest tags.
     * Default: empty set. Override to query your quest/narrative system.
     */
    virtual FGameplayTagContainer BuildActiveQuestTags(UWorld* World) const;
};
