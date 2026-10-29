#include "Conditions/FVConditionLibrary.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

FFVConditionContext UFVConditionLibrary::MakePlayerContext(UObject* WorldContext, AActor* Target)
{
	FFVConditionContext Context;
	Context.WorldContext = WorldContext;
	Context.Instigator = GetPlayerPawn(WorldContext);
	Context.Target = Target;
	return Context;
}

APawn* UFVConditionLibrary::GetPlayerPawn(const UObject* WorldContext)
{
	return UGameplayStatics::GetPlayerPawn(WorldContext, 0);
}

FFVConditionContext UFVConditionLibrary::MakeContext(AActor* Instigator, AActor* Target)
{
	FFVConditionContext Context;
	Context.Instigator = Instigator;
	Context.Target = Target;
	Context.WorldContext = Instigator != nullptr ? static_cast<UObject*>(Instigator) : Target;
	return Context;
}

bool UFVConditionLibrary::EvaluateConditionSet(const FFVConditionSet& Set, AActor* Instigator, AActor* Target)
{
	return Set.Evaluate(MakeContext(Instigator, Target));
}

FText UFVConditionLibrary::DescribeConditionSet(const FFVConditionSet& Set)
{
	return Set.GetDescription();
}

bool UFVConditionLibrary::GetConditionSetScore(const FFVConditionSet& Set, const FFVConditionContext& Context, FFVConditionScore& OutScore)
{
	return Set.GetScore(Context, OutScore);
}

TArray<int32> UFVConditionLibrary::RankConditionSets(const TArray<FFVConditionSet>& Sets, const FFVConditionContext& Context)
{
	struct FRanked
	{
		int32 Index = INDEX_NONE;
		bool bScored = false;
		FFVConditionScore Score;
	};

	TArray<FRanked> Held;
	for (int32 Index = 0; Index < Sets.Num(); ++Index)
	{
		const FFVConditionSet& Set = Sets[Index];
		if (!Set.Evaluate(Context))
		{
			continue;
		}

		FRanked& Ranked = Held.AddDefaulted_GetRef();
		Ranked.Index = Index;
		Ranked.bScored = Set.GetScore(Context, Ranked.Score);
	}

	Held.StableSort([](const FRanked& A, const FRanked& B)
	{
		if (A.bScored != B.bScored)
		{
			return A.bScored;
		}
		return A.bScored && B.Score < A.Score;
	});

	TArray<int32> Result;
	Result.Reserve(Held.Num());
	for (const FRanked& Ranked : Held)
	{
		Result.Add(Ranked.Index);
	}
	return Result;
}

bool UFVConditionLibrary::IsScoreHigher(const FFVConditionScore& A, const FFVConditionScore& B)
{
	return B < A;
}

bool UFVConditionLibrary::CompareValues(const float Current, const EFVCompare Compare, const float Value)
{
	return FVCompare::Compare(Current, Compare, Value);
}

bool UFVConditionLibrary::ScoreValue(const float Current, const EFVCompare Compare, const float Value, FFVConditionScore& OutScore)
{
	return FVCompare::Score(Current, Compare, Value, OutScore);
}

FText UFVConditionLibrary::DescribeCompare(const EFVCompare Compare, const float Value)
{
	const FText Operator = FVCompare::GetDisplayName(Compare);
	const FText Number = FText::AsNumber(Value);
	return FText::Format(NSLOCTEXT("FVConditionLibrary", "Compare", "{0} {1}"), Operator, Number);
}

void UFVConditionLibrary::ApplyEffects(const FFVEffectList& Effects, AActor* Instigator, AActor* Target)
{
	Effects.Apply(MakeContext(Instigator, Target));
}
