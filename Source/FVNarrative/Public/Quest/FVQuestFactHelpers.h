#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVQuestFactHelpers.generated.h"

/**
 * Progress of a quest, stored as the value of Fact.Quest.<ChapterId>.<QuestId>.Stage
 * and of Fact.Chapter.<ChapterId>.Stage.
 *
 * Values are sparse on purpose: new stages can be inserted between existing ones
 * without renumbering saved games, and comparisons like "Stage >= Accepted" stay valid.
 * A quest that has never started simply has no Stage fact defined.
 *
 * Failed and Completed are both terminal and mutually exclusive, which is why failure
 * is a stage rather than a separate fact: there is no way to express "completed and failed".
 */
UENUM(BlueprintType)
enum class EFVQuestStage : uint8
{
	Inactive = 0,
	Offered = 10,
	Accepted = 20,
	InProgress = 30,
	Resolving = 50,
	Failed = 90,
	Completed = 100
};

namespace FVQuestFactAspects
{
	FLICKERVOIDNARRATIVE_API extern const FName Stage;
	FLICKERVOIDNARRATIVE_API extern const FName Outcome;
	FLICKERVOIDNARRATIVE_API extern const FName Objective;
	FLICKERVOIDNARRATIVE_API extern const FName Counter;
	FLICKERVOIDNARRATIVE_API extern const FName Reached;
	FLICKERVOIDNARRATIVE_API extern const FName Completed;
}

/**
 * Builds the fact tags used by the chapter and quest systems.
 *
 * Quests nest under their chapter so a single UndefineFactsUnderTag call can reset
 * either an entire chapter or one quest:
 *   Fact.Chapter.<ChapterId>.Stage                            EFVQuestStage
 *   Fact.Chapter.<ChapterId>.Outcome
 *   Fact.Quest.<ChapterId>.<QuestId>.Stage                    EFVQuestStage
 *   Fact.Quest.<ChapterId>.<QuestId>.Outcome
 *   Fact.Quest.<ChapterId>.<QuestId>.Objective.<ObjectiveId>  undefined = not issued, 0 = active, 1 = done
 *   Fact.Quest.<ChapterId>.<QuestId>.Counter.<CounterId>
 *
 * Whether a quest is required to progress its chapter is NOT stored here. That is
 * authored data, declared by the chapter's Flow graph condition group.
 */
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVQuestFactHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Fact.Quest.<ChapterId> - the parent of every quest fact in this chapter. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeChapterQuestsTag(FName ChapterId);

	/** Fact.Quest.<ChapterId>.<QuestId> - the parent of every fact belonging to this quest. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestTag(FName ChapterId, FName QuestId);

	/** Fact.Quest.<ChapterId>.<QuestId>.Stage - holds an EFVQuestStage value. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestStageTag(FName ChapterId, FName QuestId);

	/** Fact.Quest.<ChapterId>.<QuestId>.Outcome - designer defined outcome id. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestOutcomeTag(FName ChapterId, FName QuestId);

	/** Fact.Quest.<ChapterId>.<QuestId>.Objective.<ObjectiveId> */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeObjectiveTag(FName ChapterId, FName QuestId, FName ObjectiveId);

	/** Fact.Quest.<ChapterId>.<QuestId>.Counter.<CounterId> */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static FGameplayTag MakeQuestCounterTag(FName ChapterId, FName QuestId, FName CounterId);

	/** Fact.Chapter.<ChapterId> with an optional trailing aspect such as Reached or Completed. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Chapter")
	static FGameplayTag MakeChapterTag(FName ChapterId, FName Aspect);

	/** Fact.Chapter.<ChapterId>.Stage - holds an EFVQuestStage value. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Chapter")
	static FGameplayTag MakeChapterStageTag(FName ChapterId);

	/** Fact.Chapter.<ChapterId>.Outcome - designer defined outcome id. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Chapter")
	static FGameplayTag MakeChapterOutcomeTag(FName ChapterId);

	/** Convenience wrapper so Blueprints can convert a stage enum to the int stored in the fact. */
	UFUNCTION(BlueprintPure, Category = "FlickerVoid|Facts|Quest")
	static int32 QuestStageToValue(EFVQuestStage Stage);
};
