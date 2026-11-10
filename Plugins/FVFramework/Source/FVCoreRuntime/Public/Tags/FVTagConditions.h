#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "GameplayTagContainer.h"
#include "FVTagConditions.generated.h"

/** The instigator (or the target) has the required gameplay tags and none of the blocked ones, wherever they were granted. */
USTRUCT(BlueprintType, meta = (DisplayName = "Actor Tags"))
struct FVCORERUNTIME_API FFVCondition_ActorTags : public FFVConditionBase
{
	GENERATED_BODY()

	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ToolTip = "Require every tag in RequiredTags instead of any one of them."))
	bool bRequireAll = true;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ToolTip = "Check the target instead of the instigator."))
	bool bCheckTarget = false;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};
