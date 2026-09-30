#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "FVFactConditions.generated.h"

class UFVFactDatabase;

UENUM(BlueprintType)
enum class EFVFactCompare : uint8
{
	Equal UMETA(DisplayName = "=="),
	NotEqual UMETA(DisplayName = "!="),
	Greater UMETA(DisplayName = ">"),
	GreaterOrEqual UMETA(DisplayName = ">="),
	Less UMETA(DisplayName = "<"),
	LessOrEqual UMETA(DisplayName = "<="),
	Defined UMETA(DisplayName = "defined"),
	Undefined UMETA(DisplayName = "undefined")
};

UENUM(BlueprintType)
enum class EFVFactWrite : uint8
{
	Set,
	Add,
	Remove
};

USTRUCT(BlueprintType, meta = (DisplayName = "Fact"))
struct FVCORERUNTIME_API FFVCondition_Fact : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (Categories = "Fact"))
	FGameplayTag Fact;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "Compare != EFVFactCompare::Defined && Compare != EFVFactCompare::Undefined", EditConditionHides))
	int32 Value = 1;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Fact"))
struct FVCORERUNTIME_API FFVEffect_Fact : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (Categories = "Fact"))
	FGameplayTag Fact;

	UPROPERTY(EditAnywhere, Category = "Effect")
	EFVFactWrite Write = EFVFactWrite::Set;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (EditCondition = "Write != EFVFactWrite::Remove", EditConditionHides))
	int32 Value = 1;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

namespace FVFacts
{
	FVCORERUNTIME_API bool Compare(const UFVFactDatabase& Database, FGameplayTag Fact, EFVFactCompare Compare, int32 Value);
}
