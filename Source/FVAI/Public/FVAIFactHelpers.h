#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVAIFactHelpers.generated.h"

namespace FVNpcFactAspects
{
	FLICKERVOIDAI_API extern const FName Met;
	FLICKERVOIDAI_API extern const FName Alive;
	FLICKERVOIDAI_API extern const FName Trust;
	FLICKERVOIDAI_API extern const FName Disposition;
	FLICKERVOIDAI_API extern const FName Knows;
	FLICKERVOIDAI_API extern const FName Reputation;
}

/**
 * Builds the fact tags used for NPCs and factions.
 *
 * Everything for one NPC lives under Fact.NPC.<NpcId>, so a single
 * UndefineFactsUnderTag(MakeNpcTag(NpcId)) forgets that NPC entirely.
 */
UCLASS()
class FLICKERVOIDAI_API UFVAIFactHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Fact.NPC.<NpcId> - the parent of every fact about this NPC. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|NPC")
	static FGameplayTag MakeNpcTag(FName NpcId);

	/** Fact.NPC.<NpcId>.<Aspect>, e.g. Met, Alive, Trust, Disposition. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|NPC")
	static FGameplayTag MakeNpcFactTag(FName NpcId, FName Aspect);

	/** Fact.NPC.<NpcId>.Knows.<TopicId> */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|NPC")
	static FGameplayTag MakeNpcKnowsTag(FName NpcId, FName TopicId);

	/** Fact.Faction.<FactionId>[.<Aspect>] */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|NPC")
	static FGameplayTag MakeFactionFactTag(FName FactionId, FName Aspect);
};
