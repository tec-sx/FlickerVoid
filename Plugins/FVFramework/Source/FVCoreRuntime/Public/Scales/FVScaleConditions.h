#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "FVScaleConditions.generated.h"

class UFVScaleDefinition;

/** Compares a scale's value. At-least comparisons are scored, so the hardest threshold ranks first. */
UCLASS(meta = (DisplayName = "Scale"))
class FVCORERUNTIME_API UFVCondition_Scale : public UFVCondition
{
	GENERATED_BODY()

public:
	virtual bool Evaluate_Implementation(const FFVConditionContext& Context) const override;
	virtual bool Score_Implementation(const FFVConditionContext& Context, FFVConditionScore& OutScore) const override;
	virtual FText GetDescription_Implementation() const override;

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVScaleDefinition> Scale;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVCompare Compare = EFVCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 Value = 0;
};

/** Changes a scale within its range and passes the change on through its links. */
UCLASS(meta = (DisplayName = "Modify Scale"))
class FVCORERUNTIME_API UFVEffect_ModifyScale : public UFVEffect
{
	GENERATED_BODY()

public:
	virtual void Apply_Implementation(const FFVConditionContext& Context) const override;
	virtual FText GetDescription_Implementation() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVScaleDefinition> Scale;

	UPROPERTY(EditAnywhere, Category = "Effect")
	int32 Delta = 0;
};
