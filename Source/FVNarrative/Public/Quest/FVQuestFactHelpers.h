#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVQuestFactHelpers.generated.h"

/**
 * Progress of a quest, stored as the value of Fact.Quest.<QuestId>.Stage.
 *
 * Values are sparse on purpose: new stages can be inserted between existing ones
 * without renumbering saved games, and comparisons like "Stage >= Accepted" stay valid.
 * A quest that has never started simply has no Stage fact defined.
 */
UENUM(BlueprintType)
enum class EFVQuestStage : uint8
{
	Inactive = 0,
	Offered = 10,
	Accepted = 20,
	InProgress = 30,
	Resolving = 50,
	Completed = 100
};

namespace FVQuestFactAspects
{
	FLICKERVOIDNARRATIVE_API extern const FName Stage;
	FLICKERVOIDNARRATIVE_API extern const FName Outcome;
	FLICKERVOIDNARRATIVE_API extern const FName Failed;
	FLICKERVOIDNARRATIVE_API extern const FName Objective;
	FLICKERVOIDNARRATIVE_API extern const FName Counter;
	FLICKERVOIDNARRATIVE_API extern const FName Reached;
	FLICKERVOIDNARRATIVE_API extern const FName Completed;
}

/**
 * Builds the fact tags used by the quest system.
 *
 * Layout (objectives and counters nest under their quest so a single
 * UndefineFactsUnderTag(MakeQuestTag(QuestId)) call resets an entire quest):
 *   Fact.Quest.<QuestId>.Stage
 *   Fact.Quest.<QuestId>.Outcome
 *   Fact.Quest.<QuestId>.Objective.<ObjectiveId>
 *   Fact.Quest.<QuestId>.Counter.<CounterId>
 *   Fact.Chapter.<ChapterId>[.<Aspect>]
 */
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVQuestFactHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Fact.Quest.<QuestId> - the parent of every fact belonging to this quest. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestTag(FName QuestId);

	/** Fact.Quest.<QuestId>.Stage - holds an EFVQuestStage value. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestStageTag(FName QuestId);

	/** Fact.Quest.<QuestId>.Outcome - designer defined outcome id. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestOutcomeTag(FName QuestId);

	/** Fact.Quest.<QuestId>.Failed - set when the quest ended unsuccessfully. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestFailedTag(FName QuestId);

	/** Fact.Quest.<QuestId>.Objective.<ObjectiveId> */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeObjectiveTag(FName QuestId, FName ObjectiveId);

	/** Fact.Quest.<QuestId>.Counter.<CounterId> */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestCounterTag(FName QuestId, FName CounterId);

	/** Fact.Chapter.<ChapterId> with an optional trailing aspect such as Reached or Completed. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Chapter")
	static FGameplayTag MakeChapterTag(FName ChapterId, FName Aspect);

	/** Convenience wrapper so Blueprints can convert a stage enum to the int stored in the fact. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static int32 QuestStageToValue(EFVQuestStage Stage);
};
