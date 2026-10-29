#pragma once

#include "CoreMinimal.h"
#include "FVCompare.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "Compare"))
enum class EFVCompare : uint8
{
	Equal UMETA(DisplayName = "=="),
	NotEqual UMETA(DisplayName = "!="),
	Greater UMETA(DisplayName = ">"),
	GreaterOrEqual UMETA(DisplayName = ">="),
	Less UMETA(DisplayName = "<"),
	LessOrEqual UMETA(DisplayName = "<=")
};

/** How hard an at-least condition is to meet and how far past its threshold the value is. A higher threshold ranks first. */
USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVConditionScore
{
	GENERATED_BODY()

	static FFVConditionScore AtLeast(const float Current, const float InThreshold)
	{
		FFVConditionScore Score;
		Score.Threshold = InThreshold;
		Score.Excess = Current - InThreshold;
		return Score;
	}

	bool operator<(const FFVConditionScore& Other) const { return Threshold != Other.Threshold ? Threshold < Other.Threshold : Excess < Other.Excess; }

	UPROPERTY(BlueprintReadWrite, Category = "Condition", meta = (ToolTip = "The value the condition asks for; harder conditions ask for more."))
	float Threshold = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Condition", meta = (ToolTip = "How far past the threshold the current value is."))
	float Excess = 0.f;
};

namespace FVCompare
{
	template <typename T>
	bool Compare(const T Current, const EFVCompare Op, const T Value)
	{
		switch (Op)
		{
		case EFVCompare::Equal:          return Current == Value;
		case EFVCompare::NotEqual:       return Current != Value;
		case EFVCompare::Greater:        return Current > Value;
		case EFVCompare::GreaterOrEqual: return Current >= Value;
		case EFVCompare::Less:           return Current < Value;
		case EFVCompare::LessOrEqual:    return Current <= Value;
		default:                         return false;
		}
	}

	/** Equal and NotEqual allow for float error. */
	inline bool Compare(const float Current, const EFVCompare Op, const float Value)
	{
		switch (Op)
		{
		case EFVCompare::Equal:    return FMath::IsNearlyEqual(Current, Value);
		case EFVCompare::NotEqual: return !FMath::IsNearlyEqual(Current, Value);
		default:                   return Compare<float>(Current, Op, Value);
		}
	}

	/** Scores a greater comparison (see UFVCondition::GetScore); false for the others. */
	FVCORERUNTIME_API bool Score(float Current, EFVCompare Compare, float Value, FFVConditionScore& OutScore);

	FVCORERUNTIME_API FText GetDisplayName(EFVCompare Compare);
}
