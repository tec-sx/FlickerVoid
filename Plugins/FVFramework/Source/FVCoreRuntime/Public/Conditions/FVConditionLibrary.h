#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "FVConditionLibrary.generated.h"

class APawn;

UCLASS()
class FVCORERUNTIME_API UFVConditionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Condition", meta = (DefaultToSelf = "Instigator"))
	static bool EvaluateConditionSet(const FFVConditionSet& Set, AActor* Instigator, AActor* Target);

	UFUNCTION(BlueprintPure, Category = "FV|Condition")
	static FText DescribeConditionSet(const FFVConditionSet& Set);

	UFUNCTION(BlueprintPure, Category = "FV|Condition", meta = (ToolTip = "Score of the set's at-least conditions (see Rank Condition Sets); false when none is scored."))
	static bool GetConditionSetScore(const FFVConditionSet& Set, const FFVConditionContext& Context, FFVConditionScore& OutScore);

	UFUNCTION(BlueprintPure, Category = "FV|Condition", meta = (ToolTip = "Indices of the sets that hold, hardest first: highest at-least threshold, then furthest past it. Unscored sets come last; ties keep the given order, so order the input by your tie break."))
	static TArray<int32> RankConditionSets(const TArray<FFVConditionSet>& Sets, const FFVConditionContext& Context);

	UFUNCTION(BlueprintPure, Category = "FV|Condition", meta = (ToolTip = "A is harder than B: a higher threshold, or the same threshold and further past it."))
	static bool IsScoreHigher(const FFVConditionScore& A, const FFVConditionScore& B);

	UFUNCTION(BlueprintPure, Category = "FV|Condition")
	static bool CompareValues(float Current, EFVCompare Compare, float Value);

	UFUNCTION(BlueprintCallable, Category = "FV|Effect", meta = (DefaultToSelf = "Instigator"))
	static void ApplyEffects(const FFVEffectList& Effects, AActor* Instigator, AActor* Target);

	UFUNCTION(BlueprintPure, Category = "FV|Condition", meta = (WorldContext = "WorldContext", ToolTip = "A context whose instigator is the player pawn, for rules the story checks against the player (titles, knowledge, quests, cinematics)."))
	static FFVConditionContext MakePlayerContext(UObject* WorldContext, AActor* Target = nullptr);

	static FFVConditionContext MakeContext(AActor* Instigator, AActor* Target);

	/** The pawn story rules are checked against; null before it spawns. */
	static APawn* GetPlayerPawn(const UObject* WorldContext);
};
