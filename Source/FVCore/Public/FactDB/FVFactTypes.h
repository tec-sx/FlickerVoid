#pragma once
#include "GameplayTagContainer.h"
#include "FVFactTypes.generated.h"

class UFVFactSubsystem;

UENUM(BlueprintType)
enum class EFVFactCompareOperator : uint8
{
	Equals UMETA(DisplayName = "=="),
	NotEquals UMETA(DisplayName = "!="),
	Greater UMETA(DisplayName = ">"),
	GreaterOrEqual UMETA(DisplayName = ">="),
	Less UMETA(DisplayName = "<"),
	LessOrEqual UMETA(DisplayName = "<="),
	IsUndefined UMETA(DisplayName = "undefined"),
	IsDefined UMETA(DisplayName = "defined")
};

UENUM(BlueprintType)
enum class EFVFactValueChangeType : uint8
{
	Set,
	Add
};

// Helper struct for checking single fact condition
USTRUCT(BlueprintType)
struct FFVFactCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fact", meta=(Categories="Fact"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fact")
	EFVFactCompareOperator Operator = EFVFactCompareOperator::Equals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fact")
	int32 WantedValue = 0;

	FFVFactCondition() {}

	FFVFactCondition(FGameplayTag InTag, int32 InValue, EFVFactCompareOperator InOperator)
		: Tag(InTag)
		, Operator(InOperator)
		, WantedValue(InValue)
	{
	}

	bool IsValid() const;
	FString ToString() const;

private:

	bool CheckValue(const UFVFactSubsystem& FactSubsystem) const;
};